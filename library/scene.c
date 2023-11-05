#include "scene.h"
#include "sdl_wrapper.h"
#include <stdlib.h>

int const BODY_COUNT = 200;
int const FORCES_COUNT = 200;
int const TEXT_COUNT = 10;

typedef struct scene_force {
  force_creator_t forcer;
  list_t *bodies; // doesn't own bodies
  void *aux;
  free_func_t aux_freer;
} scene_force_t;

void scene_force_free(scene_force_t *scene_force) {
  list_free(scene_force->bodies);
  if (scene_force->aux_freer != NULL && scene_force->aux != NULL) {
    scene_force->aux_freer(scene_force->aux);
  }
  free(scene_force);
}

struct scene {
  list_t *bodies;
  list_t *forces;
  list_t *texts;
  size_t width;
  size_t height;
};

scene_t *scene_init(size_t width, size_t height) {
  list_t *bodies = list_init(BODY_COUNT, (free_func_t)body_free);
  list_t *forces = list_init(FORCES_COUNT, (free_func_t)scene_force_free);
  list_t *texts = list_init(TEXT_COUNT, (free_func_t)sdl_free_text);

  scene_t *scene = malloc(sizeof(scene_t));
  scene->bodies = bodies;
  scene->forces = forces;
  scene->texts = texts;
  scene->width = width;
  scene->height = height;
  return scene;
}

void scene_free(scene_t *scene) {
  list_free(scene->forces);
  list_free(scene->bodies);
  list_free(scene->texts);
  free(scene);
}

size_t scene_bodies(scene_t *scene) { return list_size(scene->bodies); }

body_t *scene_get_body(scene_t *scene, size_t index) {
  return list_get(scene->bodies, index);
}

void scene_add_body(scene_t *scene, body_t *body) {
  list_add(scene->bodies, body);
}

void scene_remove_body(scene_t *scene, size_t index) {
  body_remove(scene_get_body(scene, index));
}

void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies,
                                    free_func_t freer) {
  scene_force_t *force = malloc(sizeof(scene_force_t));
  force->forcer = forcer;
  force->aux = aux;
  force->bodies = bodies;
  force->aux_freer = freer;

  list_add(scene->forces, force);
}

void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux,
                             free_func_t freer) {
  scene_add_bodies_force_creator(scene, forcer, aux, NULL, freer);
}

void scene_tick(scene_t *scene, double dt) {

  // apply all forces (note forces can add more forces)
  for (size_t i = 0; i < list_size(scene->forces); i++) {
    scene_force_t *force = list_get(scene->forces, i);
    force->forcer(force->aux);
  }

  // remove force_creators of marked bodies
  for (size_t j = 0; j < list_size(scene->forces); j++) {
    scene_force_t *force = list_get(scene->forces, j);
    if (force->bodies != NULL) {
      for (size_t k = 0; k < list_size(force->bodies); k++) {
        body_t *body = list_get(force->bodies, k);
        if (body_is_removed(body)) {
          list_remove(scene->forces, j);
          scene_force_free(force);
          j--;
          break;
        }
      }
    }
  }

  // update/remove bodies
  for (size_t i = scene_bodies(scene); i > 0; i--) {
    body_t *body = scene_get_body(scene, i - 1);
    body_tick(body, dt);
    if (body_is_removed(body)) {
      list_remove(scene->bodies, i - 1);
      body_free(body);
    }
  }
}

size_t scene_get_width(scene_t *scene) { return scene->width; }

size_t scene_get_height(scene_t *scene) { return scene->height; }

size_t scene_texts(scene_t *scene) { return list_size(scene->texts); }

text_t *scene_get_text(scene_t *scene, size_t index) {
  return list_get(scene->texts, index);
}

void scene_add_text(scene_t *scene, text_t *text) {
  list_add(scene->texts, text);
}

text_t *scene_remove_text(scene_t *scene, size_t index) {
  return list_remove(scene->texts, index);
}
