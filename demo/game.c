#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "body.h"
#include "collision.h"
#include "forces.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "vector.h"

const vector_t SCREEN_SIZE = {.x = 800.0, .y = 800.0};
const int CIRC_NPOINTS = 50;
const rgb_color_t PLAYER_COLOR = {1.0, 0.75, 0.8};
const double NEW_BULLET_TIME = 2.0;

const int BLOCK_ROWS = 4;
const double BLOCK_LENGTH = 74.5;
const double BLOCK_WIDTH = 20;
const double BLOCK_MASS = INFINITY;
const double BLOCK_SEPERATION = 5;
const rgb_color_t COLORS[] = {
    {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}, {1.0, 0.0, 1.0}, {1.0, 0.0, 0.0}};
char *BLOCK_TAG = "block";

const double PLAYER_LENGTH = 100;
const double PLAYER_WIDTH = 25;
const double PLAYER_H_AXIS = 400;
const double PLAYER_V_AXIS = 25;
const double PLAYER_MASS = INFINITY;
const double PLAYER_VELOCITY = 500;
char *P_TAG = "player";

const double ELASTICITY = 1.0;
const double BULLET_MASS = 50.0;
const vector_t BULLET_VELOCITY = {250, 250};
const double BULLET_V_AXIS = 50;
const double BULLET_RADIUS = 5;
char *B_TAG = "bullet";

const rgb_color_t WALL_COLOR = {1.0, 1.0, 0.0};
const double WALL_MASS = INFINITY;
const double WALL_THICKNESS = 1000000;
char *W_TAG = "wall";

body_t *generate_block(rgb_color_t color, int hp);
body_t *generate_player();
body_t *generate_proj();
body_t *generate_wall(list_t *shape);
bool stop_condition(state_t *state);
void on_key(void *state, char key, key_event_type_t type, double held_time);
void handle_wall_collision(body_t *enemy);
bool return_true();
list_t *create_rectangle();
void initialize_game(state_t *state);
void clear_game(state_t *state);

struct body_info {
  char *type;
  int health;
} typedef body_info_t;

struct state {
  scene_t *scene;
  body_t *player;
  list_t *obstacles;
  double time_since;
};

state_t *emscripten_init() {
  state_t *state = malloc(sizeof(state_t));
  sdl_init(VEC_ZERO, SCREEN_SIZE);
  initialize_game(state);
  return state;
}

void emscripten_main(state_t *state) {
  double time_elapsed = time_since_last_tick();

  // Handles keyboard input
  sdl_on_key(on_key);

  scene_tick(state->scene, time_elapsed);

  sdl_render_scene(state->scene);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  list_free(state->obstacles);
  free(state->player);
  free(state);
}

// Helper functions:
body_t *generate_block(rgb_color_t color, int hp) {
  body_info_t *block_info = malloc(sizeof(body_info_t));
  block_info->type = BLOCK_TAG;
  block_info->health = hp;
  body_t *block =
      body_init_with_info(create_rectangle(BLOCK_LENGTH, BLOCK_WIDTH),
                          BLOCK_MASS, color, block_info, free);
  body_set_velocity(block, VEC_ZERO);
  return block;
}

body_t *generate_player() {
  body_info_t *p_info = malloc(sizeof(body_info_t));
  p_info->type = P_TAG;
  p_info->health = 0;
  body_t *player =
      body_init_with_info(create_rectangle(PLAYER_LENGTH, PLAYER_WIDTH),
                          PLAYER_MASS, PLAYER_COLOR, p_info, free);
  body_set_centroid(player, (vector_t){SCREEN_SIZE.x / 2.0, PLAYER_V_AXIS});
  return player;
}

// TODO
void on_key(void *state, char key, key_event_type_t type, double held_time) {
  body_t *player = ((state_t *)state)->player;
  if (type == KEY_PRESSED) {
    switch (key) {
    case LEFT_ARROW:
      body_set_velocity(player, (vector_t){PLAYER_VELOCITY * -1, 0});
      break;
    case RIGHT_ARROW:
      body_set_velocity(player, (vector_t){PLAYER_VELOCITY, 0});
      break;
    }
  } else if (type == KEY_RELEASED) {
    body_set_velocity(player, VEC_ZERO);
  }
}

list_t *create_rectangle(double length, double width) {
  list_t *rect = list_init(4, free);
  vector_t *v = malloc(sizeof(*v));
  *v = VEC_ZERO;
  list_add(rect, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){length, 0};
  list_add(rect, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){length, width};
  list_add(rect, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){0, width};
  list_add(rect, v);
  return rect;
}

void initialize_game(state_t *state) {
  state->scene = scene_init();
  // Adds player
  body_t *player = generate_player();
  scene_add_body(state->scene, player);
  state->player = player;

  // Adds enemies
  int blocks_per_row =
      (SCREEN_SIZE.x - BLOCK_SEPERATION) / (BLOCK_LENGTH + BLOCK_SEPERATION);

  state->obstacles =
      list_init(BLOCK_ROWS * blocks_per_row, NULL); // doesn't own bodies
  double block_x = BLOCK_LENGTH / 2 + BLOCK_SEPERATION;
  double block_y = SCREEN_SIZE.y - (BLOCK_WIDTH / 2 + BLOCK_SEPERATION);
  for (int row = 0; row < BLOCK_ROWS; row++) {
    for (int column = 0; column < blocks_per_row; column++) {
      body_t *block =
          generate_block(COLORS[BLOCK_ROWS - row - 1], BLOCK_ROWS - row);
      body_set_centroid(block, (vector_t){block_x, block_y});
      scene_add_body(state->scene, block);
      list_add(state->obstacles, block);
      block_x += BLOCK_LENGTH + BLOCK_SEPERATION;
    }
    block_y -= BLOCK_WIDTH + BLOCK_SEPERATION;
    block_x = BLOCK_LENGTH / 2 + BLOCK_SEPERATION;
  }
}
