#include "block.h"
#include "body.h"
#include "list.h"
#include <math.h>
#include <stdlib.h>

const size_t CIRCLE_N_POINTS = 180;
const rgb_color_t OBSTACLE_COLOR = {0.05, 0.05, 0.05};
const double PLAYER_MASS = 10;
const double GEN_TIME = 1.0;
const int LIST_SIZE = 5;
const double ENEMY_MASS = 10;
const double BLOCK_WIDTH = 10;
const double PROJ_VELOCITY = 50;
const int SIZE_ADJ = 4;

struct obstacle_info {
  double time_since;
  double gen_time;
  list_t *projs;
};

body_t *gen_ground(int level) {
  if (level == 1) {
    return sprite_init(INFINITY, GROUND, "assets/level_1_sprites/grass.png",
                       BLOCK_WIDTH + 2, BLOCK_WIDTH + 2);
  } else if (level == 2) {
    return sprite_init(INFINITY, GROUND, "assets/level_2_sprites/sand.png",
                       BLOCK_WIDTH + 2, BLOCK_WIDTH + 2);
  }
}

body_t *gen_wall(int level) {
  if (level == 1) {
    return sprite_init(INFINITY, WALL, "assets/level_1_sprites/grass.png",
                       BLOCK_WIDTH + 2, BLOCK_WIDTH + 2);
  } else if (level == 2) {
    return sprite_init(INFINITY, WALL, "assets/level_2_sprites/sand.png",
                       BLOCK_WIDTH + 2, BLOCK_WIDTH + 2);
  }
}

body_t *gen_coin() {

  return sprite_init(INFINITY, COIN, "assets/coin.png", BLOCK_WIDTH,
                     BLOCK_WIDTH);
}

body_t *gen_player() {

  return sprite_init(PLAYER_MASS, PLAYER, "assets/duck.png", BLOCK_WIDTH,
                     BLOCK_WIDTH);
}

body_t *gen_magnet() {
  return sprite_init(INFINITY, MAGNET, "assets/spr_magnet_0.png", BLOCK_WIDTH,
                     BLOCK_WIDTH);
}

body_t *gen_fireball(int level) {
  if (level == 1) {
    body_t *f =
        sprite_init(ENEMY_MASS, FIREBALL, "assets/level_1_sprites/fireball.png",
                    SIZE_ADJ * BLOCK_WIDTH, SIZE_ADJ * BLOCK_WIDTH);
    obstacle_info_t *info = malloc(sizeof(obstacle_info_t));
    info->gen_time = 0.0;
    info->time_since = 0.0;
    info->projs = list_init(LIST_SIZE, (free_func_t)body_free);
    body_set_info(f, info);
    body_set_info_freer(f, (free_func_t)list_free);
    return f;
  } else if (level == 2) {
    body_t *f = sprite_init(ENEMY_MASS, FIREBALL,
                            "assets/level_2_sprites/waterball.png",
                            SIZE_ADJ * BLOCK_WIDTH, SIZE_ADJ * BLOCK_WIDTH);
    obstacle_info_t *info = malloc(sizeof(obstacle_info_t));
    info->gen_time = 0.0;
    info->time_since = 0.0;
    info->projs = list_init(LIST_SIZE, (free_func_t)body_free);
    body_set_info(f, info);
    body_set_info_freer(f, (free_func_t)list_free);
    return f;
  }
  return NULL;
}

body_t *gen_goomba(int level) {
  if (level == 1) {
    body_t *g =
        sprite_init(ENEMY_MASS, GOOMBA, "assets/level_1_sprites/goomba.png",
                    SIZE_ADJ * BLOCK_WIDTH, SIZE_ADJ * BLOCK_WIDTH);
    obstacle_info_t *info = malloc(sizeof(obstacle_info_t));
    info->gen_time = GEN_TIME;
    info->time_since = 0.0;
    info->projs = list_init(LIST_SIZE, (free_func_t)body_free);
    body_set_info(g, info);
    body_set_info_freer(g, (free_func_t)list_free);
    return g;
  } else if (level == 2) {
    body_t *g =
        sprite_init(ENEMY_MASS, GOOMBA, "assets/level_2_sprites/crab.png",
                    SIZE_ADJ * BLOCK_WIDTH, SIZE_ADJ * BLOCK_WIDTH);
    obstacle_info_t *info = malloc(sizeof(obstacle_info_t));
    info->gen_time = GEN_TIME;
    info->time_since = 0.0;
    info->projs = list_init(LIST_SIZE, (free_func_t)body_free);
    body_set_info(g, info);
    body_set_info_freer(g, (free_func_t)list_free);
    return g;
  }
  return NULL;
}

body_t *gen_plant(int level) {
  if (level == 1) {
    body_t *p =
        sprite_init(ENEMY_MASS, PLANT, "assets/level_1_sprites/piranha.png",
                    SIZE_ADJ * BLOCK_WIDTH, SIZE_ADJ * BLOCK_WIDTH);
    obstacle_info_t *info = malloc(sizeof(obstacle_info_t));
    info->gen_time = GEN_TIME;
    info->time_since = 0.0;
    info->projs = list_init(LIST_SIZE, (free_func_t)body_free);
    body_set_info(p, info);
    body_set_info_freer(p, (free_func_t)list_free);
    return p;
  } else if (level == 2) {
    body_t *p =
        sprite_init(ENEMY_MASS, PLANT, "assets/level_2_sprites/seaweed.png",
                    SIZE_ADJ * BLOCK_WIDTH, SIZE_ADJ * BLOCK_WIDTH);
    obstacle_info_t *info = malloc(sizeof(obstacle_info_t));
    info->gen_time = GEN_TIME;
    info->time_since = 0.0;
    info->projs = list_init(LIST_SIZE, (free_func_t)body_free);
    body_set_info(p, info);
    body_set_info_freer(p, (free_func_t)list_free);
    return p;
  }
  return NULL;
}

body_t *gen_spaceship(int level) {
  if (level == 1) {
    body_t *s =
        sprite_init(ENEMY_MASS, SPACESHIP, "assets/level_1_sprites/ufo.png",
                    SIZE_ADJ * BLOCK_WIDTH, SIZE_ADJ * BLOCK_WIDTH);
    obstacle_info_t *info = malloc(sizeof(obstacle_info_t));
    info->gen_time = GEN_TIME;
    info->time_since = 0.0;
    info->projs = list_init(LIST_SIZE, (free_func_t)body_free);
    body_set_info(s, info);
    body_set_info_freer(s, (free_func_t)list_free);
    return s;
  } else if (level == 2) {
    body_t *s = sprite_init(ENEMY_MASS, SPACESHIP,
                            "assets/level_2_sprites/submarine.png",
                            SIZE_ADJ * BLOCK_WIDTH, SIZE_ADJ * BLOCK_WIDTH);
    obstacle_info_t *info = malloc(sizeof(obstacle_info_t));
    info->gen_time = GEN_TIME;
    info->time_since = 0.0;
    info->projs = list_init(LIST_SIZE, (free_func_t)body_free);
    body_set_info(s, info);
    body_set_info_freer(s, (free_func_t)list_free);
    return s;
  }
  return NULL;
}

body_t *gen_thomp() {
  body_t *t = sprite_init(ENEMY_MASS, THOMP, "assets/level_1_sprites/thomp.png",
                          SIZE_ADJ * BLOCK_WIDTH, SIZE_ADJ * BLOCK_WIDTH);
  obstacle_info_t *info = malloc(sizeof(obstacle_info_t));
  info->gen_time = GEN_TIME;
  info->time_since = 0.0;
  info->projs = list_init(LIST_SIZE, (free_func_t)body_free);
  body_set_info(t, info);
  body_set_info_freer(t, (free_func_t)list_free);
  return t;
}

body_t *gen_portal() {
  return sprite_init(INFINITY, MAGNET, "assets/portal20.png", BLOCK_WIDTH,
                     BLOCK_WIDTH);
}

body_t *gen_ball(int level) {
  if (level == 1) {
    return sprite_init(ENEMY_MASS, SPACESHIP, "assets/level_1_sprites/ball.png",
                       BLOCK_WIDTH * 2, BLOCK_WIDTH * 2);
  } else if (level == 2) {
    return sprite_init(ENEMY_MASS, SPACESHIP,
                       "assets/level_2_sprites/water-bullet.png",
                       BLOCK_WIDTH * 2, BLOCK_WIDTH * 2);
  }
  return NULL;
}

body_t *block_init(body_type_t body_type) {

  switch (body_type) {
  case PLAYER:
    return gen_player();
    break;
  case GROUND:
    return gen_ground(1);
    break;
  case WALL:
    return gen_wall(1);
    break;
  case COIN:
    return gen_coin();
    break;
  case FIREBALL:
    return gen_fireball(1);
    break;
  case GOOMBA:
    return gen_goomba(1);
    break;
  case PLANT:
    return gen_plant(1);
    break;
  case SPACESHIP:
    return gen_spaceship(1);
    break;
  case THOMP:
    return gen_thomp();
    break;
  case MAGNET:
    return gen_magnet();
    break;
  case PORTAL:
    return gen_portal();
    break;
  case BALL:
    return gen_ball(1);
    break;
  case SAND:
    return gen_ground(2);
    break;
  case SAND_WALL:
    return gen_wall(2);
    break;
  case SEAWEED:
    return gen_plant(2);
    break;
  case SUBMARINE:
    return gen_spaceship(2);
    break;
  case WATERBALL:
    return gen_fireball(2);
    break;
  case CRAB:
    return gen_goomba(2);
    break;
  case BULLET:
    return gen_ball(2);
    break;
  default:
    return NULL;
    break;
  }
}

void block_set_pos(body_t *block, size_t x_index, size_t y_index,
                   vector_t absolute_origin) {

  float x_coord =
      x_index * BLOCK_WIDTH + BLOCK_WIDTH / 2 + 1 + absolute_origin.x;
  float y_coord =
      y_index * BLOCK_WIDTH + BLOCK_WIDTH / 2 + 1 + absolute_origin.y;

  body_set_centroid(block, (vector_t){x_coord, y_coord});
}

double block_get_time_since(body_t *block) {
  obstacle_info_t *info = body_get_info(block);
  return info->time_since;
}

double block_get_time(body_t *block) {
  obstacle_info_t *info = body_get_info(block);
  return info->gen_time;
}

void block_set_time_since(body_t *block, double time) {
  obstacle_info_t *info = body_get_info(block);
  info->gen_time = time;
}

list_t *block_get_projs(body_t *block) {
  obstacle_info_t *info = body_get_info(block);
  return info->projs;
}

void block_add_proj(body_t *block, body_t *projectile) {
  obstacle_info_t *info = body_get_info(block);
  list_add(info->projs, (body_t *)projectile);
}
