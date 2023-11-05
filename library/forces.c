#include "forces.h"
#include "collision.h"
#include "polygon.h"
#include "scene.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

const double GRAVITY_DISTANCE = 5.0;

typedef struct aux {
  list_t *bodies;
  double *doubles;

  collision_handler_t handler;
  void *secondary_aux;
  free_func_t aux_freer;
  bool collided;
} aux_t;

aux_t *aux_init(int num_bodies, int num_doubles) {
  double *doubles = malloc(num_doubles * sizeof(double));
  list_t *bodies = list_init(num_bodies, NULL);
  aux_t *aux = malloc(sizeof(aux_t));

  aux->doubles = doubles;
  aux->bodies = bodies;

  aux->secondary_aux = NULL;
  aux->aux_freer = NULL;
  aux->collided = false;
  return aux;
}

void aux_free(aux_t *aux) {
  free(aux->doubles);
  list_free(aux->bodies);
  if (aux->aux_freer != NULL && aux->secondary_aux != NULL) {
    (aux->aux_freer)(aux->secondary_aux);
  }
  free(aux);
}

/** Get the straight-line distance between two bodies
 *
 * @param body1 the first body
 * @param body2 the second body
 *
 * @return the distance between the centroids of the two bodies
 */
double get_distance(body_t *body1, body_t *body2) {
  vector_t c1 = body_get_centroid(body1);
  vector_t c2 = body_get_centroid(body2);
  return sqrt(pow(c2.x - c1.x, 2) + pow(c2.y - c1.y, 2));
}

/** Get the CCW angle from the horizontal between body1 and body2
 *
 * @param body1 the first body
 * @param body2 the second body
 *
 * @return the angle measured CCW from the horizontal between body1 and body2
 */
double get_angle(body_t *body1, body_t *body2) {
  vector_t c1 = body_get_centroid(body1);
  vector_t c2 = body_get_centroid(body2);

  return atan2(c2.y - c1.y, c2.x - c1.x);
}

void apply_newtonian_gravity(aux_t *aux) {

  // Unpack args
  body_t *body1 = list_get(aux->bodies, 0);
  body_t *body2 = list_get(aux->bodies, 1);
  double G = aux->doubles[0];

  double dist = get_distance(body1, body2);
  if (dist < GRAVITY_DISTANCE) {
    return;
  }

  float mag = G * body_get_mass(body1) * body_get_mass(body2) / pow(dist, 2);
  double angle = get_angle(body1, body2);
  vector_t v = (vector_t){.x = mag * cos(angle), .y = mag * sin(angle)};

  body_add_force(body1, v);
  body_add_force(body2, vec_negate(v));
}

void create_newtonian_gravity(scene_t *scene, double G, body_t *body1,
                              body_t *body2) {

  // Package args
  aux_t *aux = aux_init(2, 1);
  list_add(aux->bodies, body1);
  list_add(aux->bodies, body2);
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  aux->doubles[0] = G;

  scene_add_bodies_force_creator(scene,
                                 (force_creator_t)apply_newtonian_gravity,
                                 (void *)aux, bodies, (free_func_t)aux_free);
}

void apply_spring(aux_t *aux) {

  // Unpack args
  body_t *body1 = list_get(aux->bodies, 0);
  body_t *body2 = list_get(aux->bodies, 1);
  double k = aux->doubles[0];

  float mag = k * get_distance(body1, body2);
  double angle = get_angle(body1, body2);
  vector_t v = (vector_t){.x = mag * cos(angle), .y = mag * sin(angle)};

  body_add_force(body1, v);
  body_add_force(body2, vec_negate(v));
}

void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2) {

  aux_t *aux = aux_init(2, 1);
  list_add(aux->bodies, body1);
  list_add(aux->bodies, body2);
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  aux->doubles[0] = k;

  scene_add_bodies_force_creator(scene, (force_creator_t)apply_spring,
                                 (void *)aux, bodies, (free_func_t)aux_free);
}

void apply_free_on_exit(aux_t *aux) {

  body_t *body = list_get(aux->bodies, 0);
  list_t *shape = body_get_shape(body);
  for (size_t i = 0; i < body_get_size(body); i++) {
    if (((vector_t *)list_get(shape, i))->x >= 0) {
      list_free(shape);
      return;
    }
  }
  list_free(shape);
  body_remove(body);
}

void create_free_on_exit(scene_t *scene, body_t *body) {
  aux_t *aux = aux_init(1, 0);
  list_add(aux->bodies, body);
  list_t *bodies = list_init(1, NULL);
  list_add(bodies, body);
  scene_add_bodies_force_creator(scene, (force_creator_t)apply_free_on_exit,
                                 (void *)aux, bodies, (free_func_t)aux_free);
}

void apply_keep_on_screen(aux_t *aux) {
  body_t *body = list_get(aux->bodies, 0);
  double max_x = aux->doubles[0];
  double max_y = aux->doubles[1];
  list_t *shape = body_get_shape(body);
  vector_t current_centroid = polygon_centroid(shape);
  for (size_t i = 0; i < body_get_size(body); i++) {
    vector_t current_pos = *((vector_t *)list_get(shape, i));

    if (current_pos.x < 0) {
      vector_t new_centroid =
          (vector_t){current_centroid.x - current_pos.x, current_centroid.y};
      body_set_centroid(body, new_centroid);
    } else if (current_pos.x > max_x) {
      vector_t new_centroid = (vector_t){
          current_centroid.x - current_pos.x + max_x, current_centroid.y};
      body_set_centroid(body, new_centroid);
    }

    if (current_pos.y < 0) {
      vector_t new_centroid =
          (vector_t){current_centroid.x, current_centroid.y - current_pos.y};
      body_set_centroid(body, new_centroid);
    } else if (current_pos.x > max_y) {
      vector_t new_centroid = (vector_t){
          current_centroid.x, current_centroid.y - current_pos.y + max_y};
      body_set_centroid(body, new_centroid);
    }

    break;
  }
}

void create_keep_on_screen(scene_t *scene, double max_x, double max_y,
                           body_t *body) {
  aux_t *aux = aux_init(1, 2);
  list_add(aux->bodies, body);
  list_t *bodies = list_init(1, NULL);
  list_add(bodies, body);
  aux->doubles[0] = max_x;
  aux->doubles[1] = max_y;
  scene_add_bodies_force_creator(scene, (force_creator_t)apply_keep_on_screen,
                                 (void *)aux, bodies, (free_func_t)aux_free);
}

void apply_drag(aux_t *aux) {

  // Unpack args
  body_t *body = list_get(aux->bodies, 0);
  double gamma = aux->doubles[0];

  body_add_force(body, vec_multiply(-1 * gamma, body_get_velocity(body)));
}

void create_drag(scene_t *scene, double gamma, body_t *body) {

  aux_t *aux = aux_init(1, 1);
  list_add(aux->bodies, body);
  list_t *bodies = list_init(1, NULL);
  list_add(bodies, body);
  aux->doubles[0] = gamma;
  scene_add_bodies_force_creator(scene, (force_creator_t)apply_drag,
                                 (void *)aux, bodies, (free_func_t)aux_free);
}

void apply_gravity(aux_t *aux) {

  // Unpack args
  body_t *body = list_get(aux->bodies, 0);
  double g = aux->doubles[0];

  body_add_force(body, (vector_t){0, -body_get_mass(body) * g});
}

void create_gravity(scene_t *scene, double gravity, body_t *body) {

  aux_t *aux = aux_init(1, 1);
  list_add(aux->bodies, body);
  list_t *bodies = list_init(1, NULL);
  list_add(bodies, body);
  aux->doubles[0] = gravity;
  scene_add_bodies_force_creator(scene, (force_creator_t)apply_gravity,
                                 (void *)aux, bodies, (free_func_t)aux_free);
}

void handle_destructive_collision(body_t *body1, body_t *body2, vector_t axis,
                                  void *aux) {
  body_remove(body1);
  body_remove(body2);
}

void create_destructive_collision(scene_t *scene, body_t *body1,
                                  body_t *body2) {
  aux_t *aux = aux_init(2, 0);
  list_add(aux->bodies, body1);
  list_add(aux->bodies, body2);

  create_collision(scene, body1, body2,
                   (collision_handler_t)handle_destructive_collision,
                   (void *)aux, (void *)aux_free);
}

void physics_handler(body_t *body1, body_t *body2, vector_t axis, void *aux) {

  list_t *shape1 = body_get_shape(body1);
  list_t *shape2 = body_get_shape(body2);

  double m1 = body_get_mass(body1);
  double m2 = body_get_mass(body2);
  double v1 = vec_dot(body_get_velocity(body1), axis);
  double v2 = vec_dot(body_get_velocity(body2), axis);

  double elasticity = ((double *)aux)[0];

  double reduced_mass;
  if (m1 == INFINITY) {
    reduced_mass = m2;
  } else if (m2 == INFINITY) {
    reduced_mass = m1;
  } else {
    reduced_mass = (m1 * m2) / (m1 + m2);
  }

  double impulse = reduced_mass * (1 + elasticity) * (v2 - v1);
  vector_t impulse_vec = vec_multiply(impulse, axis);
  body_add_impulse(body1, impulse_vec);
  body_add_impulse(body2, vec_negate(impulse_vec));

  list_free(shape1);
  list_free(shape2);
}

void create_physics_collision(scene_t *scene, double elasticity, body_t *body1,
                              body_t *body2) {
  double *aux = malloc(sizeof(double));
  *aux = elasticity;
  create_collision(scene, body1, body2, physics_handler, aux, free);
}

void collision_force_creator(aux_t *aux) {

  body_t *body1 = list_get(aux->bodies, 0);
  body_t *body2 = list_get(aux->bodies, 1);

  list_t *shape1 = body_get_shape(body1);
  list_t *shape2 = body_get_shape(body2);

  collision_info_t col = find_collision(shape1, shape2);

  collision_handler_t handler = aux->handler;

  if (col.collided && !aux->collided) {
    handler(body1, body2, col.axis, aux->secondary_aux);
    aux->collided = true;
  } else {
    aux->collided = false;
  }

  list_free(shape1);
  list_free(shape2);
}

void create_collision(scene_t *scene, body_t *body1, body_t *body2,
                      collision_handler_t handler, void *aux,
                      free_func_t freer) {
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);

  aux_t *a = aux_init(2, 0);
  list_add(a->bodies, body1);
  list_add(a->bodies, body2);
  a->handler = handler;
  a->secondary_aux = aux;
  a->aux_freer = freer;

  scene_add_bodies_force_creator(scene,
                                 (force_creator_t)collision_force_creator, a,
                                 bodies, (free_func_t)aux_free);
}
