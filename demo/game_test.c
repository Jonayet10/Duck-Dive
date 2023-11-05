#include <SDL2/SDL_mixer.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "block.h"
#include "body.h"
#include "collision.h"
#include "forces.h"
#include "level.h"
#include "list.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "vector.h"

const size_t VIEWPORT_WIDTH = 800;
const vector_t SCENE_SIZE = {800, 800};
const double SCROLL_SPEED = 100;
const double JUMP_VELOCITY = 200;
const double DOUBLE_JUMP_VELOCITY = 150;
const double PLAYER_VELOCITY = 200;
const double INVINSIBILTY_TICKS = 100;
const double HEART_SIZE = 50;
const vector_t PORTAL_DISPLACEMENT = {300, 100};

const double GRAVITY = 250;
const double GROUND_PADDING = 1.5;
const double WALL_PADDING = 3;
const int COIN_PLAYER_DIST = 100;
const double MAGNET_STRENGTH = 100.0;
const double PROJECTILE_VELOCITY = 100;
const double OBSTACLE_MASS = 10;
const size_t NUM_LEVEL_BUTTON = 1;

// Text Constants
const rgb_color_t RED = {1.0, 0.0, 0.0};
const rgb_color_t GREEN = {0.0, 1.0, 0.0};
const rgb_color_t BLACK = {0.0, 0.0, 0.0};
const char *ORANGE_TTF = "/assets/fonts/orange.ttf";
const char *SQUID_TTF = "/assets/fonts/Squid.ttf";
const size_t HEAD_PT = 100;
const size_t SUBHEAD_PT = 60;
const size_t TEXT_PT = 50;
const size_t BUTTON_PT = 50;
const size_t COSMETICS_PT = 24;

// Button Types
const vector_t LARGE_BUTTON = {300, 70};
const vector_t MEDIUM_BUTTON = {150, 70};
const vector_t SMALL_SQUARE_BUTTON = {25, 25};
const vector_t LARGE_SQUARE_BUTTON = {100, 100};
const char *BUTTON_IMG = "assets/sprites/button.png";

// Menu Constansts
const vector_t TITLE_POS = {400, 700};
const size_t NUM_MENU_BUTTON = 2;
const vector_t LEVELS_POS = {400, 400};
const vector_t COSMETICS_POS = {400, 320};
const vector_t LEADERBOARD_POS = {400, 240};
const char *MENU_BG = "assets/sprites/grass_bg.png";
// Level Select
const int NUM_LEVELS = 2;
const vector_t LEVEL1_POS = {300, 400};
const vector_t LEVEL2_POS = {500, 400};
const char *LEVELS_BG = "assets/sprites/grass_bg.png";
// COSMETICS
const int NUM_COSMETICS = 6;
const int COSMETICS_ROWS = 2;
const int COSMETICS_COLUMNS = 2;
// 2 times space between buttons
const double COSMETICS_PADDING = 20;
const double TEXT_OFFSET = 10;
// first button position
const vector_t CUSTOMIZE_POS = {100, 400};
const vector_t CUSTOMIZE_MENU_POS = {600, 400};
const vector_t SCORE_POS = {400, 600};
const char *COSMETICS[6] = {"assets/duck.png",
                            "assets/cosmetics/unlock_1.png",
                            "assets/cosmetics/unlock_2.png",
                            "assets/cosmetics/unlock_3.png",
                            "assets/cosmetics/unlock_4.png",
                            "assets/cosmetics/unlock_5.png"};
const int COSMETICS_COST[6] = {0, 10, 10, 10, 10, 10};
// Used to store text for purchase buttons and score
const int CUSTOMIZE_BUFFER = 10;

// Leaderboard Constants
const vector_t LB_LEVEL1_POS = {200, 600};
const vector_t LB_LEVEL2_POS = {600, 600};
const vector_t LB_MENU_POS = {400, 100};
const vector_t LEVEL1_SCORE_POS = {200, 350};
const vector_t LEVEL2_SCORE_POS = {600, 350};
const vector_t LEVEL1_START = {200, 500};
const vector_t LEVEL2_START = {600, 500};
const char *LB_BG_PATH = "assets/sprites/lb_bg3.png";
const char *LB_TEXT_BG_PATH = "assets/white_box.png";
const int HEADER_WIDTH = 550;
const int HEADER_HEIGHT = 100;
const int SUBHEADER_WIDTH = 250;
const int SUBHEADER_HEIGHT = 80;
const int LB_SIZE = 10;
const int LB_BUFFER = 100;
const int SCORE_HEIGHT = 400;
const int LB_DIST = 75;

// Pause Constants
const size_t NUM_PAUSE_BUTTON = 4;
const vector_t PAUSE_TEXT_POS = {400, 400};
const vector_t GAME_BUTTON_POS = {400, 180};
const vector_t MENU_BUTTON_POS = {400, 260};
const vector_t PAUSE_BUTTON_POS = {13, 787};
const char *PAUSE_IMG = "assets/sprites/pause_button.png";
const char *PAUSE_BG = "assets/sprites/duck_bg.png";

typedef enum {
  MENU,
  PAUSED,
  LEVELS,
  GAME,
  CUSTOMIZE,
  GAMEOVER,
  LEADERBOARD
} active_t;

// Pause Constants
const size_t NUM_GAMEOVER_BUTTON = 1;
const vector_t GAMEOVER_POS = {400, 400};
const char *GAMEOVER_BG = "assets/sprites/duck_bg.png";

body_t *jump_block(body_t *player);
void unpause(state_t *state);
void music_init(state_t *state);
void music_free(state_t *state);
void ground_handler(body_t *player, body_t *ground, vector_t axis, void *state);
void wall_handler(body_t *player, body_t *wall, vector_t axis, void *state);
void gen_hearts(scene_t *scene);
void lower_health(body_t *player, body_t *enemy, vector_t axis, void *state);
void magnet_handler(body_t *player, body_t *magnet, vector_t axis,
                    state_t *state);
void coin_collector(body_t *player, body_t *coin, vector_t axis, void *aux);
void load_force(state_t *state, body_t *body);
void portal_handler(body_t *player, body_t *portal, vector_t axis, void *state);

typedef void (*button_handler_t)(state_t *state);
typedef void (*button_handler_with_idx_t)(state_t *state, int idx);
void play_buzzer(state_t *state);
void menu_init(state_t *state);
void level_select_init(state_t *state);
void level1_init(state_t *state);
void game_over(state_t *state);
void level2_init(state_t *state);
void level_complete(state_t *state, int idx);
void purchase(state_t *state, int idx);

struct body_info {
  int health;
  int coin_count;
  int jumps;
} typedef body_info_t;

struct button_info {
  button_handler_with_idx_t handler;
  int idx;
} typedef button_info_t;

struct state {
  scene_t *scene;
  body_t *player;
  active_t active;
  scene_t *paused_scene;
  list_t *buttons;
  list_t *pause_buttons;
  list_t *customize_buttons;
  list_t *customize_text;
  bool game_started;

  list_t *level;
  vector_t absolute_origin;
  int last_column_loaded;
  double ticks_since_damage;

  bool level1_complete;
  bool level2_complete;
  int score;
  bool *unlocks;
  int equipped_cosmetic_idx;

  list_t *level1_scores;
  list_t *level2_scores;

  Mix_Music *background_music;
  Mix_Chunk *quack_effect;
  Mix_Chunk *buzzer;
};

// Hearts of size 50x50
// Creates hearts with sprite_init and sets their centroid to top right of
// screen
void gen_hearts(scene_t *scene) {
  body_t *heart_three =
      sprite_init(INFINITY, HEART3, "assets/heart.png", HEART_SIZE, HEART_SIZE);
  body_set_centroid(heart_three, (vector_t){SCENE_SIZE.x - 2 * HEART_SIZE,
                                            SCENE_SIZE.y - 2 * HEART_SIZE});
  body_t *heart_two =
      sprite_init(INFINITY, HEART2, "assets/heart.png", HEART_SIZE, HEART_SIZE);
  body_set_centroid(heart_two, (vector_t){SCENE_SIZE.x - 4 * HEART_SIZE,
                                          SCENE_SIZE.y - 2 * HEART_SIZE});
  body_t *heart_one =
      sprite_init(INFINITY, HEART1, "assets/heart.png", HEART_SIZE, HEART_SIZE);
  body_set_centroid(heart_one, (vector_t){SCENE_SIZE.x - 6 * HEART_SIZE,
                                          SCENE_SIZE.y - 2 * HEART_SIZE});

  scene_add_body(scene, heart_one);
  scene_add_body(scene, heart_two);
  scene_add_body(scene, heart_three);
}

body_t *gen_button(vector_t size, vector_t pos, const char *sprite_path,
                   button_handler_t handler) {
  body_t *button = sprite_init(INFINITY, OTHER, sprite_path, size.x, size.y);
  body_set_info(button, handler);
  body_set_centroid(button, pos);
  return button;
}

body_t *gen_button_with_idx(vector_t size, vector_t pos,
                            const char *sprite_path,
                            button_handler_with_idx_t handler, int idx) {
  body_t *button = sprite_init(INFINITY, OTHER, sprite_path, size.x, size.y);
  button_info_t *info = malloc(sizeof(*info));
  info->handler = handler;
  info->idx = idx;
  body_set_info(button, (void *)info);
  body_set_centroid(button, pos);
  return button;
}

/**
 * Pauses game.
 */
void pause(state_t *state) {
  scene_t *pause = scene_init(SCENE_SIZE.x, SCENE_SIZE.y);
  // Background
  body_t *bg =
      sprite_init(INFINITY, BACKGROUND, PAUSE_BG, SCENE_SIZE.x, SCENE_SIZE.y);
  scene_add_body(pause, bg);
  // Text
  text_t *head =
      sdl_create_text("PAUSED", ORANGE_TTF, HEAD_PT, RED, PAUSE_TEXT_POS);
  scene_add_text(pause, head);
  // Buttons
  state->pause_buttons = list_init(NUM_PAUSE_BUTTON, NULL);
  body_t *button =
      gen_button(LARGE_BUTTON, GAME_BUTTON_POS, BUTTON_IMG, unpause);
  text_t *text = sdl_create_text("Back", ORANGE_TTF, BUTTON_PT, BLACK,
                                 body_get_centroid(button));
  scene_add_text(pause, text);
  scene_add_body(pause, button);
  list_add(state->pause_buttons, button);

  button = gen_button(LARGE_BUTTON, MENU_BUTTON_POS, BUTTON_IMG, menu_init);
  text = sdl_create_text("Menu", ORANGE_TTF, BUTTON_PT, BLACK,
                         body_get_centroid(button));
  scene_add_body(pause, button);
  scene_add_text(pause, text);
  list_add(state->pause_buttons, button);
  // Swap
  state->paused_scene = state->scene;
  state->scene = pause;
  state->active = PAUSED;
}

/**
 * Unpauses game.
 */
void unpause(state_t *state) {
  scene_free(state->scene);
  state->scene = state->paused_scene;
  list_free(state->pause_buttons);
  state->active = GAME;
}

void play_buzzer(state_t *state) { Mix_PlayChannel(-1, state->buzzer, 0); }

/**
 * Purchase or equips a given customization.
 */
void purchase(state_t *state, int idx) {
  list_t *texts = state->customize_text;
  if ((state->unlocks)[idx]) {
    sdl_swap_text(list_get(texts, state->equipped_cosmetic_idx), "Owned",
                  SQUID_TTF, COSMETICS_PT, GREEN);
    state->equipped_cosmetic_idx = idx;
    sdl_swap_text(list_get(texts, idx), "Equipped", SQUID_TTF, COSMETICS_PT,
                  GREEN);
  } else if (state->score >= COSMETICS_COST[idx]) {
    (state->unlocks)[idx] = true;
    state->score -= COSMETICS_COST[idx];
    sdl_swap_text(list_get(texts, idx), "Owned", SQUID_TTF, COSMETICS_PT,
                  GREEN);
    char *buffer = malloc(CUSTOMIZE_BUFFER * sizeof(char));
    sprintf(buffer, "Score: %d", state->score);
    sdl_swap_text(list_get(texts, NUM_COSMETICS), buffer, ORANGE_TTF, BUTTON_PT,
                  RED);
  } else {
    play_buzzer(state);
  }
}

/** Clears fields in state relating to the active scene.
 *  Used before initializing a new scene.
 */
void clear_scene(state_t *state) {
  if (state->active == PAUSED) {
    unpause(state);
  }
  if (state->active == GAME) {
    list_free(state->level);
  }
  if (state->active == CUSTOMIZE) {
    list_free(state->customize_buttons);
    list_free(state->customize_text);
  }
  scene_free(state->scene);
  list_free(state->buttons);
}

void level_select_init(state_t *state) {
  clear_scene(state);
  state->scene = scene_init(SCENE_SIZE.x, SCENE_SIZE.y);
  state->active = LEVELS;
  // Background
  body_t *bg =
      sprite_init(INFINITY, BACKGROUND, MENU_BG, SCENE_SIZE.x, SCENE_SIZE.y);
  scene_add_body(state->scene, bg);
  // Text
  text_t *head = sdl_create_text("Levels", ORANGE_TTF, HEAD_PT, RED, TITLE_POS);
  scene_add_text(state->scene, head);
  // Buttons
  state->buttons = list_init(NUM_LEVELS, NULL);
  // Level 1
  body_t *button =
      gen_button(MEDIUM_BUTTON, LEVEL1_POS, BUTTON_IMG, level1_init);
  text_t *text = sdl_create_text("Level 1", ORANGE_TTF, BUTTON_PT, BLACK,
                                 body_get_centroid(button));
  scene_add_text(state->scene, text);
  scene_add_body(state->scene, button);
  list_add(state->buttons, button);
  // Level 2
  if (state->level1_complete) {
    button = gen_button(MEDIUM_BUTTON, LEVEL2_POS, BUTTON_IMG, level1_init);
    text = sdl_create_text("Level 2", ORANGE_TTF, BUTTON_PT, BLACK,
                           body_get_centroid(button));
  } else {
    button = gen_button(MEDIUM_BUTTON, LEVEL2_POS, BUTTON_IMG, play_buzzer);
    text = sdl_create_text("Locked", ORANGE_TTF, BUTTON_PT, BLACK,
                           body_get_centroid(button));
  }
  scene_add_text(state->scene, text);
  scene_add_body(state->scene, button);
  list_add(state->buttons, button);
}

void cosmetics_init(state_t *state) {
  clear_scene(state);
  state->scene = scene_init(SCENE_SIZE.x, SCENE_SIZE.y);
  state->active = CUSTOMIZE;
  // Background
  body_t *bg =
      sprite_init(INFINITY, BACKGROUND, PAUSE_BG, SCENE_SIZE.x, SCENE_SIZE.y);
  scene_add_body(state->scene, bg);
  // Header
  text_t *head =
      sdl_create_text("Cosmetics", ORANGE_TTF, HEAD_PT, RED, TITLE_POS);
  scene_add_text(state->scene, head);
  // Generate Buttons and associated text
  state->customize_buttons = list_init(NUM_COSMETICS, NULL);
  state->customize_text = list_init(NUM_COSMETICS + 1, NULL);
  body_t *button;
  text_t *text;
  char *buffer = malloc(CUSTOMIZE_BUFFER * sizeof(char));
  vector_t pos = CUSTOMIZE_POS;
  for (size_t i = 0; i < NUM_COSMETICS; i++) {
    button = gen_button_with_idx(LARGE_SQUARE_BUTTON, pos, COSMETICS[i],
                                 purchase, i);
    rgb_color_t color;
    if (state->equipped_cosmetic_idx == i) {
      sprintf(buffer, "Equipped");
      color = GREEN;
    } else if ((state->unlocks)[i]) {
      sprintf(buffer, "Owned");
      color = GREEN;
    } else {
      sprintf(buffer, "Buy: %d coins", COSMETICS_COST[i]);
      color = RED;
    }
    text = sdl_create_text(
        buffer, SQUID_TTF, COSMETICS_PT, color,
        (vector_t){pos.x, pos.y - LARGE_SQUARE_BUTTON.y / 2 - TEXT_OFFSET});
    list_add(state->customize_buttons, button);
    scene_add_body(state->scene, button);
    list_add(state->customize_text, text);
    scene_add_text(state->scene, text);

    pos.x += LARGE_SQUARE_BUTTON.x + COSMETICS_PADDING;
    if ((i + 1) % COSMETICS_COLUMNS == 0) {
      pos.x -= COSMETICS_COLUMNS * (LARGE_SQUARE_BUTTON.x + COSMETICS_PADDING);
      pos.y -= LARGE_SQUARE_BUTTON.y + COSMETICS_PADDING;
    }
  }
  // Score display
  sprintf(buffer, "Score : %d", state->score);
  text = sdl_create_text(buffer, ORANGE_TTF, BUTTON_PT, RED, SCORE_POS);
  list_add(state->customize_text, text);
  scene_add_text(state->scene, text);
  free(buffer);
  // Return to menu
  state->buttons = list_init(NUM_LEVEL_BUTTON, NULL);
  button = gen_button(LARGE_BUTTON, CUSTOMIZE_MENU_POS, BUTTON_IMG, menu_init);
  text = sdl_create_text("Menu", ORANGE_TTF, BUTTON_PT, BLACK,
                         body_get_centroid(button));
  scene_add_text(state->scene, text);
  scene_add_body(state->scene, button);
  list_add(state->buttons, button);
}

list_t *descending_sort(list_t *list) {
  size_t size = list_size(list);
  int curr;
  int num_array[size];
  // Copy over
  for (size_t i = 0; i < size; i++) {
    size_t num = list_get(list, i);
    num_array[i] = num;
  }
  // Sort num_array
  for (size_t j = 0; j < size; j++) {
    for (size_t k = j + 1; k < size; k++) {
      if (num_array[j] < num_array[k]) {
        curr = num_array[j];
        num_array[j] = num_array[k];
        num_array[k] = curr;
      }
    }
  }
  list_t *new_list = list_init(size, NULL);
  for (size_t l = 0; l < size; l++) {
    list_add(new_list, num_array[l]);
  }
  return new_list;
}

void leaderboard_init(state_t *state) {
  clear_scene(state);
  state->scene = scene_init(SCENE_SIZE.x, SCENE_SIZE.y);
  state->active = LEADERBOARD;

  // Background
  body_t *bg =
      sprite_init(INFINITY, BACKGROUND, LB_BG_PATH, SCENE_SIZE.x, SCENE_SIZE.y);
  scene_add_body(state->scene, bg);

  // Header
  text_t *head =
      sdl_create_text("High Scores", ORANGE_TTF, HEAD_PT, BLACK, TITLE_POS);
  scene_add_text(state->scene, head);
  body_t *head_txt_bg = sprite_init(INFINITY, BACKGROUND, LB_TEXT_BG_PATH,
                                    HEADER_WIDTH, HEADER_HEIGHT);
  body_set_centroid(head_txt_bg, TITLE_POS);
  scene_add_body(state->scene, head_txt_bg);

  // Subheaders
  text_t *level1_head =
      sdl_create_text("Level One", ORANGE_TTF, SUBHEAD_PT, RED, LB_LEVEL1_POS);
  scene_add_text(state->scene, level1_head);
  body_t *level1_head_bg = sprite_init(INFINITY, BACKGROUND, LB_TEXT_BG_PATH,
                                       SUBHEADER_WIDTH, SUBHEADER_HEIGHT);
  body_set_centroid(level1_head_bg, LB_LEVEL1_POS);
  scene_add_body(state->scene, level1_head_bg);
  text_t *level2_head = sdl_create_text("Level Two", ORANGE_TTF, SUBHEAD_PT,
                                        GREEN, LB_LEVEL2_POS);
  scene_add_text(state->scene, level2_head);
  body_t *level2_head_bg = sprite_init(INFINITY, BACKGROUND, LB_TEXT_BG_PATH,
                                       SUBHEADER_WIDTH, SUBHEADER_HEIGHT);
  body_set_centroid(level2_head_bg, LB_LEVEL2_POS);
  scene_add_body(state->scene, level2_head_bg);

  // Level 1 scores
  body_t *level1_scores_bg = sprite_init(INFINITY, BACKGROUND, LB_TEXT_BG_PATH,
                                         SUBHEADER_WIDTH, SCORE_HEIGHT);
  body_set_centroid(level1_scores_bg, LEVEL1_SCORE_POS);
  scene_add_body(state->scene, level1_scores_bg);
  if (list_size(state->level1_scores) == 0) {
    text_t *lvl1_lb =
        sdl_create_text("1. N/A", SQUID_TTF, TEXT_PT, RED, LEVEL1_START);
    scene_add_text(state->scene, lvl1_lb);
  } else {
    list_t *level_1_sorted = descending_sort(state->level1_scores);
    for (size_t i = 0; i < list_size(level_1_sorted); i++) {
      int score = list_get(level_1_sorted, i);
      char *buffer = malloc(LB_BUFFER * sizeof(char));
      sprintf(buffer, "%i. %i", i + 1, score);
      vector_t next_loc =
          vec_add(LEVEL1_START, (vector_t){0, -1 * LB_DIST * i});
      text_t *text = sdl_create_text(buffer, SQUID_TTF, TEXT_PT, RED, next_loc);
      scene_add_text(state->scene, text);
      free(buffer);
    }
  }

  // Level 2 scores
  body_t *level2_scores_bg = sprite_init(INFINITY, BACKGROUND, LB_TEXT_BG_PATH,
                                         SUBHEADER_WIDTH, SCORE_HEIGHT);
  body_set_centroid(level2_scores_bg, LEVEL2_SCORE_POS);
  scene_add_body(state->scene, level2_scores_bg);
  if (list_size(state->level2_scores) == 0) {
    text_t *lvl2_lb =
        sdl_create_text("1. N/A", SQUID_TTF, TEXT_PT, GREEN, LEVEL2_START);
    scene_add_text(state->scene, lvl2_lb);
  } else {
    list_t *level_2_sorted = descending_sort(state->level2_scores);
    for (size_t i = 0; i < list_size(level_2_sorted); i++) {
      int score = list_get(level_2_sorted, i);
      char *buffer = malloc(LB_BUFFER * sizeof(char));
      sprintf(buffer, "%i. %i", i + 1, score);
      vector_t next_loc =
          vec_add(LEVEL2_START, (vector_t){0, -1 * LB_DIST * i});
      text_t *text =
          sdl_create_text(buffer, SQUID_TTF, TEXT_PT, GREEN, next_loc);
      scene_add_text(state->scene, text);
      free(buffer);
    }
  }

  // Return to menu
  body_t *button;
  text_t *text;
  state->buttons = list_init(NUM_LEVEL_BUTTON, NULL);
  button = gen_button(LARGE_BUTTON, LB_MENU_POS, BUTTON_IMG, menu_init);
  text = sdl_create_text("Menu", ORANGE_TTF, BUTTON_PT, BLACK,
                         body_get_centroid(button));
  scene_add_text(state->scene, text);
  scene_add_body(state->scene, button);
  list_add(state->buttons, button);
}

void menu_init(state_t *state) {
  if (state->game_started) {
    clear_scene(state);
  }
  state->scene = scene_init(SCENE_SIZE.x, SCENE_SIZE.y);
  state->game_started = true;
  state->active = MENU;
  // Background
  body_t *bg =
      sprite_init(INFINITY, BACKGROUND, MENU_BG, SCENE_SIZE.x, SCENE_SIZE.y);
  scene_add_body(state->scene, bg);
  // Title
  text_t *head =
      sdl_create_text("Duck Dive", ORANGE_TTF, HEAD_PT, RED, TITLE_POS);
  scene_add_text(state->scene, head);
  // Buttons
  state->buttons = list_init(NUM_MENU_BUTTON, NULL);
  body_t *button =
      gen_button(LARGE_BUTTON, LEVELS_POS, BUTTON_IMG, level_select_init);
  text_t *text = sdl_create_text("Play", ORANGE_TTF, BUTTON_PT, BLACK,
                                 body_get_centroid(button));
  scene_add_text(state->scene, text);
  scene_add_body(state->scene, button);
  list_add(state->buttons, button);

  button = gen_button(LARGE_BUTTON, COSMETICS_POS, BUTTON_IMG, cosmetics_init);
  text = sdl_create_text("Shop", ORANGE_TTF, BUTTON_PT, BLACK,
                         body_get_centroid(button));
  scene_add_text(state->scene, text);
  scene_add_body(state->scene, button);
  list_add(state->buttons, button);

  button =
      gen_button(LARGE_BUTTON, LEADERBOARD_POS, BUTTON_IMG, leaderboard_init);
  text = sdl_create_text("Leaderboard", ORANGE_TTF, BUTTON_PT, BLACK,
                         body_get_centroid(button));
  scene_add_text(state->scene, text);
  scene_add_body(state->scene, button);
  list_add(state->buttons, button);
}

void level1_init(state_t *state) {
  clear_scene(state);
  state->scene = scene_init(SCENE_SIZE.x, SCENE_SIZE.y);
  state->level = load_level(state->scene, "/assets/levels/level_1.txt");
  state->absolute_origin = VEC_ZERO;

  // Render the visible map
  state->player = render_scene(state->scene, state->level, SCROLL_SPEED);
  state->last_column_loaded = VIEWPORT_WIDTH / BLOCK_WIDTH;
  state->active = GAME;

  // Buttons
  state->buttons = list_init(NUM_LEVEL_BUTTON, NULL);
  body_t *button =
      gen_button(SMALL_SQUARE_BUTTON, PAUSE_BUTTON_POS, PAUSE_IMG, pause);
  scene_add_body(state->scene, button);
  list_add(state->buttons, button);

  // Adds info to player
  body_t *player = state->player;
  body_set_texture(player, COSMETICS[state->equipped_cosmetic_idx]);
  body_set_info_freer(player, free);
  body_info_t *player_info = malloc(sizeof(body_info_t));
  player_info->health = 1;
  player_info->coin_count = 0;
  body_set_info(player, player_info);

  // Add forces and collisions
  create_gravity(state->scene, GRAVITY, player);
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    load_force(state, scene_get_body(state->scene, i));
  }

  // Adds health indicators
  gen_hearts(state->scene);
}

void level2_init(state_t *state) {}

/**
 * Completes the level and returns to the menu.
 * Should only be called on succesful level completion.
 */
void level_complete(state_t *state, int level_idx) {
  body_info_t *info = body_get_info(state->player);
  state->score += info->coin_count;
  if (level_idx == 1) {
    state->level1_complete = true;
    list_add(state->level1_scores, info->coin_count);
  }
  if (level_idx == 2) {
    state->level2_complete = true;
    list_add(state->level2_scores, info->coin_count);
  }
  menu_init(state);
}

/**
 * Closes the level and initiates game over scene.
 */
void game_over(state_t *state) {
  clear_scene(state);
  state->scene = scene_init(SCENE_SIZE.x, SCENE_SIZE.y);
  // Background
  body_t *bg = sprite_init(INFINITY, BACKGROUND, GAMEOVER_BG, SCENE_SIZE.x,
                           SCENE_SIZE.y);
  scene_add_body(state->scene, bg);
  // Title
  text_t *head =
      sdl_create_text("GAMEOVER", ORANGE_TTF, HEAD_PT, RED, TITLE_POS);
  scene_add_text(state->scene, head);
  // Buttons
  state->buttons = list_init(NUM_GAMEOVER_BUTTON, NULL);
  body_t *button =
      gen_button(LARGE_BUTTON, GAMEOVER_POS, BUTTON_IMG, menu_init);
  text_t *text = sdl_create_text("MENU", ORANGE_TTF, BUTTON_PT, BLACK,
                                 body_get_centroid(button));
  scene_add_text(state->scene, text);
  scene_add_body(state->scene, button);
  list_add(state->buttons, button);
}

void load_force(state_t *state, body_t *body) {
  body_t *player = state->player;
  scene_t *scene = state->scene;
  body_type_t type = body_get_type(body);
  if (type == MAGNET) {
    create_collision(scene, player, body, (collision_handler_t)magnet_handler,
                     NULL, NULL);
  }
  if (type == FIREBALL || type == GOOMBA || type == THOMP || type == PLANT ||
      type == SPACESHIP) {
    create_collision(scene, player, body, (collision_handler_t)lower_health,
                     state, NULL);
  }
  if (type == GROUND) {
    create_collision(scene, player, body, (collision_handler_t)ground_handler,
                     state, NULL);
  }
  if (type == WALL) {
    create_collision(scene, player, body, (collision_handler_t)wall_handler,
                     NULL, NULL);
  }
  if (type == COIN) {
    create_collision(scene, player, body, (collision_handler_t)coin_collector,
                     NULL, NULL);
  }
  if (type == PORTAL) {
    create_collision(scene, player, body, (collision_handler_t)portal_handler,
                     NULL, NULL);
  }
}

/** Collision handler to handle magnet powerup */
void portal_handler(body_t *player, body_t *portal, vector_t axis,
                    void *state) {
  vector_t centroid = body_get_centroid(player);
  body_set_centroid(player, vec_add(centroid, PORTAL_DISPLACEMENT));
}

/** Collision handler to handle magnet powerup */
void magnet_handler(body_t *player, body_t *magnet, vector_t axis,
                    state_t *state) {
  for (size_t i = 0; i < scene_bodies(((state_t *)state)->scene); i++) {
    body_t *body = scene_get_body(((state_t *)state)->scene, i);
    body_type_t type = body_get_type(body);
    if (type == COIN) {
      double dist = get_distance(player, body);
      if (dist < 5.0) {
        return;
      }
      if (get_distance(player, body) <= COIN_PLAYER_DIST) {
        double angle = get_angle(player, body);
        vector_t v = (vector_t){.x = MAGNET_STRENGTH * cos(angle),
                                .y = MAGNET_STRENGTH * sin(angle)};
        body_add_force(body, v);
      }
    }
  }
}

/** Collision handler to colllect coins */
void coin_collector(body_t *player, body_t *coin, vector_t axis, void *aux) {
  body_info_t *info = body_get_info(player);
  info->coin_count++;
  body_remove(coin);
  body_set_info(player, info);
  body_remove(coin);
}

/** Collision handler to decrease player health upon collision with enemy*/
void lower_health(body_t *player, body_t *enemy, vector_t axis, void *state) {
  state_t *state1 = state;
  if (state1->ticks_since_damage < INVINSIBILTY_TICKS) {
    return;
  }
  body_info_t *info = body_get_info(player);
  if (info->health == 1) {
    for (size_t i = 0; i < scene_bodies(((state_t *)state)->scene); i++) {
      state1->ticks_since_damage = 0;
      body_t *body = scene_get_body(((state_t *)state)->scene, i);
      body_type_t type = body_get_type(body);
      if (type == HEART1) {
        body_remove(body);
        break;
      }
    }
    info->health = info->health + 1;
    body_set_info(player, info);
    return;
  }

  if (info->health == 2) {
    state1->ticks_since_damage = 0;
    for (size_t i = 0; i < scene_bodies(((state_t *)state)->scene); i++) {
      body_t *body = scene_get_body(((state_t *)state)->scene, i);
      body_type_t type = body_get_type(body);
      if (type == HEART2) {
        body_remove(body);
        break;
      }
    }
    info->health = info->health + 1;
    body_set_info(player, info);
    return;
  }

  if (info->health == 3) {
    state1->ticks_since_damage = 0;
    for (size_t i = 0; i < scene_bodies(((state_t *)state)->scene); i++) {
      body_t *body = scene_get_body(((state_t *)state)->scene, i);
      body_type_t type = body_get_type(body);
      if (type == HEART3) {
        body_remove(body);
        break;
      }
    }
    info->health = info->health + 1;
    body_set_info(player, info);
    return;
  }

  if (info->health == 4) {
    state1->active = GAMEOVER;
  }
}

/** Collision handler to reset jump of player upon collision with obstacle and
 * keep player above ground*/
void ground_handler(body_t *player, body_t *ground, vector_t axis,
                    void *state) {
  body_info_t *info = body_get_info(player);
  info->jumps = 2;

  // Stop player from sinking into ground
  vector_t centroid = body_get_centroid(player);
  vector_t velocity = body_get_velocity(player);
  centroid.y = body_get_centroid(ground).y + BLOCK_WIDTH + GROUND_PADDING;
  velocity.y = 0;
  body_set_centroid(player, centroid);
  body_set_velocity(player, velocity);
}

/** Keeps player out of walls. */
void wall_handler(body_t *player, body_t *wall, vector_t axis, void *state) {
  vector_t wall_center = body_get_centroid(wall);
  vector_t player_center = body_get_centroid(player);
  vector_t offset = {BLOCK_WIDTH + WALL_PADDING, 0};
  vector_t v = body_get_velocity(player);
  if (wall_center.x - WALL_PADDING - BLOCK_WIDTH < player_center.x &&
      wall_center.x - WALL_PADDING > player_center.x) {
    body_set_velocity(player, (vector_t){-SCROLL_SPEED, v.y});
    body_set_centroid(player, vec_subtract(wall_center, offset));
  } else if (wall_center.x + BLOCK_WIDTH + WALL_PADDING > player_center.x) {
    body_set_velocity(player, (vector_t){-SCROLL_SPEED, v.y});
    body_set_centroid(player, vec_add(wall_center, offset));
  }
}

void on_key_1(void *state, char key, key_event_type_t type, double held_time,
              vector_t click) {
  state_t *state1 = (state_t *)state;
  body_t *player;
  vector_t v;
  body_info_t *info;
  if (state1->active == GAME) {
    player = ((state_t *)state)->player;
    v = body_get_velocity(player);
    info = body_get_info(player);
  }
  if (type == KEY_PRESSED) {
    switch (key) {
    case ESCAPE_KEY:
      if (state1->active == PAUSED) {
        unpause(state1);
      } else if (state1->active == GAME) {
        pause(state1);
      }
      break;
    case LEFT_ARROW:
      if (state1->active == GAME) {
        body_set_velocity(player,
                          (vector_t){PLAYER_VELOCITY * -1 - SCROLL_SPEED, v.y});
        body_set_flipped(player, true);
      }
      break;
    case RIGHT_ARROW:
      if (state1->active == GAME) {
        body_set_velocity(player,
                          (vector_t){PLAYER_VELOCITY - SCROLL_SPEED, v.y});
        body_set_flipped(player, false);
      }
      break;
    case SPACE_BAR:
    case UP_ARROW:
      if (state1->active == GAME) {
        if (info->jumps == 2) {
          body_set_velocity(player, (vector_t){v.x, JUMP_VELOCITY});
          Mix_PlayChannel(-1, ((state_t *)state)->quack_effect, 0);
        } else if (info->jumps == 1) {
          body_set_velocity(player, (vector_t){v.x, DOUBLE_JUMP_VELOCITY});
          Mix_PlayChannel(-1, ((state_t *)state)->quack_effect, 0);
        }
        info->jumps--;
      }
      break;
    default:
      break;
    }
  }
  if (type == KEY_RELEASED) {
    switch (key) {
    case LEFT_ARROW:
    case RIGHT_ARROW:
      body_set_velocity(player, (vector_t){-SCROLL_SPEED, v.y});
      break;
    default:
      break;
    }
  }
  if (type == MOUSE_CLICK) {
    list_t *buttons;
    if (state1->active == PAUSED) {
      buttons = state1->pause_buttons;
    } else {
      buttons = state1->buttons;
    }
    for (size_t i = 0; i < list_size(buttons); i++) {
      body_t *button = list_get(buttons, i);
      list_t *shape = body_get_shape(button);
      if (button_is_clicked(shape, click)) {
        // Only one button can be handled per click
        ((button_handler_t)body_get_info(button))(state1);
        break;
      }
      list_free(shape);
    }
    if (state1->active == CUSTOMIZE) {
      buttons = state1->customize_buttons;
      for (size_t i = 0; i < list_size(buttons); i++) {
        body_t *button = list_get(buttons, i);
        list_t *shape = body_get_shape(button);
        if (button_is_clicked(shape, click)) {
          // Only one button can be handled per click
          button_info_t *info = body_get_info(button);
          (info->handler)(state1, info->idx);
          break;
        }
        list_free(shape);
      }
    }
  }
}

body_t *generate_proj(body_t *start) {
  body_t *bullet = block_init(BALL);
  body_set_centroid(bullet, body_get_centroid(start));
  body_set_velocity(bullet, (vector_t){0, -1 * PROJECTILE_VELOCITY});
  block_add_proj(start, bullet);
  return bullet;
}

void obstacle_handler(state_t *state) {
  for (int i = 0; i < scene_bodies(state->scene); i++) {
    body_t *body = scene_get_body(state->scene, i);
    body_type_t type = body_get_type(body);
    if (type == THOMP) {
      // If block hits the ground
      // Change velocity to go upwards again
    } else if (type == GOOMBA) {
      return;
    } else if (type == SPACESHIP) {
      if (block_get_time_since(body) > block_get_time(body)) {
        block_set_time_since(body, 0.0);
        vector_t curr_velocity = body_get_velocity(body);
        body_set_velocity(body, vec_multiply(-1, curr_velocity));
        body_t *bullet = generate_proj(body);
        scene_add_body(state->scene, bullet);
      } else if (block_get_time_since(body) > block_get_time(body)) {
        body_t *bullet = generate_proj(body);
        scene_add_body(state->scene, bullet);
      }
    }
  }
}

state_t *emscripten_init() {
  state_t *state = malloc(sizeof(state_t));
  sdl_init(VEC_ZERO, SCENE_SIZE);
  state->game_started = false;
  state->level1_complete = false;
  state->level2_complete = false;
  state->score = 0;
  state->unlocks = malloc(NUM_COSMETICS * sizeof(bool));
  (state->unlocks)[0] = true;
  for (size_t i = 1; i < NUM_COSMETICS; i++) {
    (state->unlocks)[i] = false;
  }
  state->equipped_cosmetic_idx = 0;
  state->level1_scores = list_init(LB_SIZE, NULL);
  state->level2_scores = list_init(LB_SIZE, NULL);
  state->ticks_since_damage = 0;
  menu_init(state);
  sdl_on_key(on_key_1);
  music_init(state);
  return state;
}

void emscripten_main(state_t *state) {
  double time_elapsed = time_since_last_tick();

  // Scroll screen
  if (state->active == GAME) {
    // Render and forces to bodies coming on screen
    int column_to_load =
        (int)floor(-1 * state->absolute_origin.x / BLOCK_WIDTH) + 1 +
        VIEWPORT_WIDTH / BLOCK_WIDTH;
    if (column_to_load > state->last_column_loaded) {
      render_info_t *render_info =
          render_column(state->scene, state->level, column_to_load,
                        SCROLL_SPEED, state->absolute_origin);
      list_t *bodies = render_info->rendered;
      for (size_t i = 0; i < list_size(bodies); i++) {
        load_force(state, list_get(bodies, i));
      }
      render_info_free(render_info);
      state->last_column_loaded = column_to_load;
    }

    scene_tick(state->scene, time_elapsed);
    state->ticks_since_damage++;
    state->absolute_origin.x -= SCROLL_SPEED * time_elapsed;

    // Increments time
    for (int i = 0; i < scene_bodies(state->scene); i++) {
      body_t *obstacle = scene_get_body(state->scene, i);
      body_type_t type = body_get_type(obstacle);
      if (type == GOOMBA || type == SPACESHIP || type == THOMP) {
        block_set_time_since(obstacle,
                             block_get_time_since(obstacle) + time_elapsed);
      }
    }

    // Handles obstacle movements
    obstacle_handler(state);
  }
  if (state->active == GAMEOVER) {
    game_over(state);
  }
  sdl_render_scene(state->scene);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state->unlocks);
  list_free(state->buttons);
  list_free(state->pause_buttons);
  list_free(state->customize_buttons);
  list_free(state->customize_text);
  music_free(state);
  free(state);
}

void music_init(state_t *state) {
  // initialize SDL mixer
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
  // load audio files
  Mix_Music *background_music =
      Mix_LoadMUS("assets/TownTheme.wav"); // change to wav file

  Mix_VolumeMusic(MIX_MAX_VOLUME / 20);
  // start the music
  Mix_PlayMusic(background_music, -1);

  state->background_music = background_music;

  Mix_Chunk *quack = Mix_LoadWAV("assets/7HS2REN-duck-quack.wav");
  state->quack_effect = quack;

  Mix_Chunk *buzzer = Mix_LoadWAV("assets/sounds/buzzer_x.wav");
  state->buzzer = buzzer;
}

void music_free(state_t *state) {
  Mix_FreeMusic(state->background_music);
  Mix_FreeChunk(state->quack_effect);
  Mix_CloseAudio();
}
