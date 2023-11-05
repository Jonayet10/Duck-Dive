#include "body.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_surface.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const double MAX_ELASTICITY = 1.0;
const double MIN_ELASTICITY = 0.8;

struct body {
  list_t *shape;
  rgb_color_t color;
  double mass;
  double angle;
  vector_t centroid;
  vector_t velocity;
  vector_t force;
  vector_t impulse;
  double angvel;
  float elasticity;
  body_type_t body_type;
  void *info;
  free_func_t info_freer;
  bool marked_for_removal;
  SDL_Texture *texture;
  bool flipped;
};

body_t *body_init(list_t *shape, double mass, rgb_color_t color,
                  body_type_t body_type) {
  body_t *body = malloc(sizeof(body_t));
  body->shape = shape;
  body->mass = mass;
  body->color = color;
  body->angle = 0;
  body->angvel = 0;
  body->velocity = VEC_ZERO;
  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
  body->centroid = polygon_centroid(body->shape);
  body->elasticity =
      fmod(rand(), MAX_ELASTICITY - MIN_ELASTICITY) + MIN_ELASTICITY;
  body->body_type = body_type;
  body->info = NULL;
  body->info_freer = NULL;
  body->marked_for_removal = false;
  body->texture = NULL;
  body->flipped = false;
  return body;
}

body_t *sprite_init(double mass, body_type_t body_type,
                    const char *texture_path, int width, int height) {
  body_t *body = malloc(sizeof(body_t));
  body->mass = mass;
  body->color = (rgb_color_t){0, 0, 0};
  body->angle = 0;
  body->angvel = 0;
  body->velocity = VEC_ZERO;
  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
  body->elasticity =
      fmod(rand(), MAX_ELASTICITY - MIN_ELASTICITY) + MIN_ELASTICITY;
  body->body_type = body_type;
  body->marked_for_removal = false;
  body->texture = sdl_create_texture(texture_path);
  body->info = NULL;
  body->info_freer = NULL;
  body->flipped = false;

  list_t *points = list_init(4, free);
  vector_t *side1 = malloc(sizeof(vector_t));
  vector_t *side2 = malloc(sizeof(vector_t));
  vector_t *side3 = malloc(sizeof(vector_t));
  vector_t *side4 = malloc(sizeof(vector_t));

  *side1 = (vector_t){0, 0};
  *side2 = (vector_t){0, height};
  *side3 = (vector_t){width, height};
  *side4 = (vector_t){width, 0};

  list_add(points, side1);
  list_add(points, side2);
  list_add(points, side3);
  list_add(points, side4);

  body->shape = points;
  body->centroid = polygon_centroid(points);
  return body;
}

body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color,
                            body_type_t type, void *info,
                            free_func_t info_freer) {
  body_t *b = body_init(shape, mass, color, type);
  b->info = info;
  b->info_freer = info_freer;
  return b;
}

void body_free(body_t *body) {
  list_free(body->shape);
  if (body->info_freer != NULL) {
    body->info_freer(body->info);
  }
  if (body->texture) {
    SDL_DestroyTexture(body->texture);
  }
  free(body);
}

list_t *body_get_shape(body_t *body) {
  list_t *list = body->shape;
  list_t *returned = list_init(list_size(list), free);
  for (size_t i = 0; i < list_size(list); i++) {
    vector_t *curr = list_get(list, i);
    vector_t *copy = malloc(sizeof(*curr));
    *copy = *curr;
    list_add(returned, copy);
  }
  return returned;
}

vector_t body_get_centroid(body_t *body) { return body->centroid; }

vector_t body_get_velocity(body_t *body) { return body->velocity; }

double body_get_elasticity(body_t *body) { return body->elasticity; }

double body_get_angle(body_t *body) { return body->angle; }

double body_get_angvel(body_t *body) { return body->angvel; }

int body_get_size(body_t *body) { return list_size(body->shape); }

rgb_color_t body_get_color(body_t *body) { return body->color; }

double body_get_mass(body_t *body) { return body->mass; }

void body_set_centroid(body_t *body, vector_t x) {
  polygon_translate(body->shape, vec_negate(body->centroid));
  polygon_translate(body->shape, x);
  body->centroid = x;
}

void body_set_velocity(body_t *body, vector_t v) { body->velocity = v; }

void body_set_angvel(body_t *body, double angvel) { body->angvel = angvel; }

void body_set_rotation(body_t *body, double angle) {
  if (!body->texture) {
    polygon_rotate(body->shape, -(body->angle), body->centroid);
    polygon_rotate(body->shape, angle, body->centroid);
  }
  body->angle = angle;
}

void body_add_force(body_t *body, vector_t force) {
  body->force = vec_add(body->force, force);
}

void body_add_impulse(body_t *body, vector_t impulse) {
  if (body->mass != INFINITY) {
    body->impulse = vec_add(body->impulse, impulse);
  }
}

void body_set_color(body_t *body, rgb_color_t color) { body->color = color; }

void body_tick(body_t *body, double dt) {
  vector_t acceleration = vec_multiply(1.0 / body->mass, body->force);
  vector_t curr_vel = body->velocity;
  vector_t new_vel = vec_add(curr_vel, vec_multiply(dt, acceleration));
  new_vel = vec_add(new_vel, vec_multiply(1.0 / body->mass, body->impulse));
  vector_t avg_vel = vec_multiply(0.5, vec_add(curr_vel, new_vel));
  vector_t dx = vec_multiply(dt, avg_vel);

  body->centroid = vec_add(body->centroid, dx);
  polygon_translate(body->shape, dx);

  double rotate = body->angvel * dt;
  body->angle += rotate;

  if (!body->texture) {
    polygon_rotate(body->shape, rotate, body->centroid);
  }

  body->velocity = new_vel;
  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
}

bool body_is_removed(body_t *body) { return body->marked_for_removal; }

void body_remove(body_t *body) { body->marked_for_removal = true; }

body_type_t body_get_type(body_t *body) { return body->body_type; }

void *body_get_info(body_t *body) { return body->info; }

void body_set_info(body_t *body, void *info) { body->info = info; }

void body_set_info_freer(body_t *body, free_func_t info_freer) {
  body->info_freer = free;
}

void body_set_flipped(body_t *body, bool flipped) { body->flipped = flipped; }

bool body_get_flipped(body_t *body) { return body->flipped; }

SDL_Texture *body_get_texture(body_t *body) { return body->texture; }

void body_set_texture(body_t *body, char *texture_path) {
  SDL_DestroyTexture(body->texture);
  body->texture = sdl_create_texture(texture_path);
}
