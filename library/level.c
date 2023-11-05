#include "level.h"
#include "block.h"
#include "body.h"
#include "forces.h"
#include "list.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int ASCII_INT_CONVERSION = -48;
const double PROJ_SPEED = 200;

const double SCREEN_WIDTH = 800.0;
const double SCREEN_HEIGHT = 800.0;

int parse_int(char c) { return (int)c + ASCII_INT_CONVERSION; }

/**
 * Converts text file to bodies at correct
 * position on the grid.
 *
 * @param scene scene to add bodies to
 * @param level_txt file containing level design
 * @return list of bodies with correct position
 */
list_t *load_bodies(scene_t *scene, char *level_txt) {
  // Check format
  char *header = malloc(5 * sizeof(char));
  header[4] = '\0';
  FILE *file = fopen(level_txt, "r");
  assert(file != NULL);
  size_t read = fread(header, sizeof(*header), 4, file);
  assert(read);
  assert(!strcmp(header, "dim:"));
  free(header);

  // Get dimensions
  char *dims = malloc(sizeof(char));
  size_t length = 0;
  size_t height = 0;
  while (fread(dims, sizeof(*dims), 1, file) && *dims != 'x') {
    length = 10 * length + parse_int(*dims);
  }
  while (fread(dims, sizeof(*dims), 1, file) && *dims != '\n') {
    height = 10 * height + parse_int(*dims);
  }
  free(dims);

  list_t *level = list_init(height, (free_func_t)list_free);
  for (size_t i = 0; i < height; i++) {
    list_t *r = list_init(length, (free_func_t)free);
    list_add(level, r);
  }

  // Get bodies
  char *block = malloc(sizeof(char));
  int row = height - 1;

  list_t *row_list = list_get(level, row);
  while (fread(block, sizeof(*block), 1, file)) {
    body_type_t *body_type = malloc(sizeof(body_type_t));
    *body_type = AIR;
    switch (*block) {
    case 'p':
      *body_type = PLAYER;
      break;
    case 'w':
      *body_type = WALL;
      break;
    case 'g':
      *body_type = GROUND;
      break;
    case 'c':
      *body_type = COIN;
      break;
    case 'f':
      *body_type = FIREBALL;
      break;
    case 'o':
      *body_type = GOOMBA;
      break;
    case 'l':
      *body_type = PLANT;
      break;
    case 's':
      *body_type = SPACESHIP;
      break;
    case 't':
      *body_type = THOMP;
      break;
    case 'm':
      *body_type = MAGNET;
      break;
    case 'x':
      *body_type = PORTAL;
      break;
    case 'a':
      *body_type = SAND;
      break;
    case 'i':
      *body_type = SEAWEED;
      break;
    case 'u':
      *body_type = SUBMARINE;
      break;
    case 'e':
      *body_type = WATERBALL;
      break;
    case 'r':
      *body_type = CRAB;
      break;
    case 'y':
      *body_type = FLAG;
      break;
    case '\n':
      row--;
      if (row >= 0) {
        row_list = list_get(level, row);
      }
      break;
    default:
      break;
    }
    if (*block != '\n') {
      list_add(row_list, body_type);
    }
  }
  return level;
}

list_t *load_level(scene_t *scene, char *level_txt) {
  return load_bodies(scene, level_txt);
}

body_t *render_scene(scene_t *scene, list_t *level, double scroll_speed) {
  size_t width = scene_get_width(scene);
  body_t *player;

  body_t *bg = sprite_init(INFINITY, BACKGROUND,
                           "/assets/level_1_sprites/level1_bg.png", 2125, 800);
  body_set_velocity(bg, (vector_t){scroll_speed / -4.0, 0});
  scene_add_body(scene, bg);
  for (int i = 0; i < width / BLOCK_WIDTH + 1; i++) {
    render_info_t *info =
        render_column(scene, level, i, scroll_speed, VEC_ZERO);
    if (info->player != NULL) {
      player = info->player;
    }
    render_info_free(info);
  }
  assert(player != NULL);
  create_keep_on_screen(scene, SCREEN_WIDTH, SCREEN_HEIGHT, player);
  return player;
}

render_info_t *render_column(scene_t *scene, list_t *level, size_t column,
                             double scroll_speed, vector_t absolute_origin) {
  body_t *player = NULL; // stores player if in column
  list_t *bodies = list_init(list_size(level), NULL);
  if (column >= list_size(list_get(level, 0))) {
    return NULL;
  }
  for (size_t i = 0; i < list_size(level); i++) {
    list_t *row_list = list_get(level, i);
    body_type_t body_type = *(body_type_t *)list_get(row_list, column);
    if (body_type != AIR) {
      body_t *block = block_init(body_type);
      create_free_on_exit(scene, block);
      list_add(bodies, block);
      block_set_pos(block, column, i, absolute_origin);
      body_set_velocity(block, (vector_t){-1 * scroll_speed, 0});
      scene_add_body(scene, block);
      if (body_type == PLAYER) {
        player = block;
      } else if (body_type == FIREBALL || body_type == WATERBALL) {
        body_set_velocity(block, (vector_t){-1 * PROJ_SPEED, 0});
      } else if (body_type == GOOMBA || body_type == CRAB) {
        obstacle_info_t *info = body_get_info(block);
      } else if (body_type == THOMP) {
        body_set_velocity(block, (vector_t){0, -1 * PROJ_SPEED});
      } else if (body_type == SPACESHIP || body_type == SUBMARINE) {
        body_set_velocity(block, (vector_t){-1 * PROJ_SPEED, 0});
      }
    }
  }
  render_info_t *returned = malloc(sizeof(*returned));
  returned->player = player;
  returned->rendered = bodies;

  return returned;
}

void render_info_free(render_info_t *info) {
  list_free(info->rendered);
  free(info);
}
