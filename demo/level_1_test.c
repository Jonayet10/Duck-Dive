#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "block.h"
#include "body.h"
#include "collision.h"
#include "forces.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "vector.h"

const size_t VIEWPORT_WIDTH = 800;
const vector_t SCENE_SIZE = {800, 800};

const int BLOCK_ROWS = 4;
const double BLOCK_LENGTH = 74.5;
const double BLOCK_W = 20;
const double BLOCK_MASS = INFINITY;
const double BLOCK_SEPERATION = 5;
char *BLOCK_TAG = "block";

const double PLAYER_LENGTH = 100;
const double PLAYER_WIDTH = 100;
const double PLAYER_VELOCITY = 500;
rgb_color_t PLAYER_COLOR = {0.0, 1.0, 0.0};

const double ELASTICITY = 0;
const double OBSTACLE_MASS = 20.0;
const double OBSTACLE_LENGTH = 20;
const double OBSTACLE_W = 25;
const vector_t OBSTACLE_VELOCITY = {-250, 0};
rgb_color_t OBSTACLE_RGB = {1.0, 0.0, 0.0};
const double DOUBLE_JUMP_INTERVAL = 0.5; // 500 ms
const double OBS_GEN_TIME = 5;

const rgb_color_t WALL_COLOR = {1.0, 1.0, 0.0};
const double WALL_MASS = INFINITY;
const double WALL_THICKNESS = 1000000;
const int ACCELERATION = 70;
char *W_TAG = "wall";

const vector_t JUMP_IMPULSE = {0, 1500};
const vector_t GRAV_IMPULSE = {0, -1500};
const vector_t DOUBLE_JUMP_IMPULSE = {0, 1000};
const double LEFT = M_PI;
const double DOWN = 3 * M_PI / 2;
const double RIGHT = 0.0;

list_t *create_rectangle(double length, double width);
void on_key(void *state, char key, key_event_type_t type, double held_time,
            vector_t click);
void background_music_init(state_t *state);
void background_music_free(state_t *state);
void game_init(state_t *state);
void background_init(state_t *state);
void background_free(state_t *state);
body_t *generate_obstacle();
body_t *generate_wall(list_t *shape);

struct state {
  body_t *player;
  scene_t *scene;
  list_t *obstacles;
  double time_since_last_jump;
  arrow_key_t last_press;
  Mix_Music *background_music;
  double time_since_last_obstacle;
  SDL_Surface *background;
};

state_t *emscripten_init() {
  state_t *state = malloc(sizeof(state_t));
  sdl_init(VEC_ZERO, (vector_t){VIEWPORT_WIDTH, SCENE_SIZE.y});

  background_music_init(state);
  game_init(state);
  background_init(state);
  return state;
}

void emscripten_main(state_t *state) {
  double time_elapsed = time_since_last_tick();
  state->time_since_last_obstacle += time_elapsed;
  if (state->time_since_last_obstacle >= OBS_GEN_TIME) {
    body_t *obs = generate_obstacle();
    scene_add_body(state->scene, obs);
  }
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
  background_music_free(state);
}

void on_key(void *state, char key, key_event_type_t type, double held_time,
            vector_t click) {
  body_t *player = ((state_t *)state)->player;
  vector_t velocity = body_get_velocity(player);
  if (type == KEY_PRESSED) {
    switch (key) {
    case LEFT_ARROW:
      velocity.y = 0;
      body_set_velocity(player, (vector_t){PLAYER_VELOCITY * -1, 0});
      body_set_rotation(player, LEFT);
      ((state_t *)state)->last_press = LEFT_ARROW;
      break;
    case RIGHT_ARROW:
      velocity.y = 0;
      body_set_velocity(player, (vector_t){PLAYER_VELOCITY, 0});
      body_set_rotation(player, RIGHT);
      ((state_t *)state)->last_press = RIGHT_ARROW;
      break;
    case SPACE_BAR:
      ((state_t *)state)->time_since_last_jump += time_since_last_tick();
      if ((((state_t *)state)->time_since_last_jump < DOUBLE_JUMP_INTERVAL)) {
        body_add_impulse(player, DOUBLE_JUMP_IMPULSE);
        ((state_t *)state)->time_since_last_jump = 0;
      } else if ((((state_t *)state)->time_since_last_jump >
                  DOUBLE_JUMP_INTERVAL) ||
                 (((state_t *)state)->time_since_last_jump == 0)) {
        ((state_t *)state)->time_since_last_jump = 0;
        body_add_impulse(player, JUMP_IMPULSE);
        break;
      }
      ((state_t *)state)->last_press = SPACE_BAR;
    }
  } else if (type == KEY_RELEASED) {
    body_set_velocity(player, VEC_ZERO);
    body_add_impulse(player, GRAV_IMPULSE);
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

body_t *generate_obstacle() {
  list_t *o = create_rectangle(OBSTACLE_LENGTH, OBSTACLE_W);
  body_t *obst = body_init(o, OBSTACLE_MASS, OBSTACLE_RGB, OBSTACLE);
  body_set_velocity(obst, OBSTACLE_VELOCITY);
  body_set_centroid(obst, (vector_t){SCENE_SIZE.x, OBSTACLE_W / 2});
  return obst;
}

body_t *generate_wall(list_t *shape) {
  body_t *wall = body_init(shape, WALL_MASS, WALL_COLOR, WALL);
  return wall;
}

void game_init(state_t *state) {
  state->last_press = LEFT_ARROW;
  state->time_since_last_obstacle = OBS_GEN_TIME;

  // Add player
  // state->player = body_init(create_rectangle(PLAYER_LENGTH, PLAYER_WIDTH),
  //                           PLAYER_MASS, PLAYER_COLOR, OTHER);
  body_set_centroid(state->player,
                    (vector_t){PLAYER_LENGTH / 2, PLAYER_WIDTH / 2});
  state->scene = scene_init((size_t)SCENE_SIZE.x, (size_t)SCENE_SIZE.y);
  scene_add_body(state->scene, state->player);

  // Add Walls
  list_t *top_shape = create_rectangle(SCENE_SIZE.x, WALL_THICKNESS);
  body_t *top = generate_wall(top_shape);
  body_set_centroid(
      top, (vector_t){SCENE_SIZE.x / 2, SCENE_SIZE.y + WALL_THICKNESS / 2});
  scene_add_body(state->scene, top);

  list_t *left_shape = create_rectangle(WALL_THICKNESS, SCENE_SIZE.y);
  body_t *left = generate_wall(left_shape);
  body_set_centroid(left, (vector_t){-WALL_THICKNESS / 2, SCENE_SIZE.y / 2});
  scene_add_body(state->scene, left);

  list_t *right_shape = create_rectangle(WALL_THICKNESS, SCENE_SIZE.y);
  body_t *right = generate_wall(right_shape);
  body_set_centroid(
      right, (vector_t){SCENE_SIZE.x + WALL_THICKNESS / 2, SCENE_SIZE.y / 2});
  scene_add_body(state->scene, right);

  list_t *bottom_shape = create_rectangle(WALL_THICKNESS, SCENE_SIZE.y);
  body_t *bottom = generate_wall(bottom_shape);
  body_set_centroid(bottom, (vector_t){SCENE_SIZE.x / 2, -SCENE_SIZE.y / 2});
  scene_add_body(state->scene, bottom);

  // Add collsions
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    body_t *body = scene_get_body(state->scene, i);
    body_type_t type = body_get_type(body);
    if (type == WALL) {
      create_physics_collision(state->scene, ELASTICITY, state->player, body);
    }
  }
}

void background_music_init(state_t *state) {
  // initialize SDL mixer
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
  // load audio files
  Mix_Music *background_music =
      Mix_LoadMUS("assets/TownTheme.wav"); // change to wav file

  Mix_VolumeMusic(MIX_MAX_VOLUME / 20);
  // start the music
  Mix_PlayMusic(background_music, -1);

  state->background_music = background_music;
}

void background_music_free(state_t *state) {
  Mix_FreeMusic(state->background_music);
}

void background_init(state_t *state) {
  SDL_Surface *image;
  SDL_RWops *rwop;
  rwop = SDL_RWFromFile("bg.jpg", "rb");
  image = IMG_LoadJPG_RW(rwop);
  if (!image) {
    printf("IMG_LoadJPG_RW: %s\n", IMG_GetError());
    // handle error
  }
  state->background = image;
}
void background_free(state_t *state) { IMG_Quit(); }
