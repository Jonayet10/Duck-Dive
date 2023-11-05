#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "body.h"
#include "list.h"
#include "scene.h"

/**
 * Struct to store information from render_column
 */
typedef struct render_info {
  body_t *player;
  // list of rendered bodies
  list_t *rendered;
} render_info_t;

/**
 * Adds bodies to a level as designated by the blocks
 * in the level design.
 *
 * @param scene scene to add bodies to
 * @param text file containing level design
 *
 * @return the list of bodies in the scene
 */
list_t *load_level(scene_t *scene, char *level_txt);

/**
 * Renders the part of the map initially visible on screen.
 * Asserts the initial scene contains a player.
 * @param scene the scene to be rendered.
 * @param level list containing body types of bodys to be rendered,
 *              returned from load_level
 * @param scroll_speed scroll speed of the level.
 * @returns the player on the screen.
 */
body_t *render_scene(scene_t *scene, list_t *level, double scroll_speed);

/**
 * Renders a single column of a level
 * @param scene the scene to be rendered.
 * @param level list containing body types of bodys to be rendered,
 *              returned from load_level
 * @param coumn index of column in level list.
 * @param scroll_speed scroll speed of the level.
 * @param absolut_origin location of the origin (changes due to scrolling).
 * @returns render info containing the player if the column contains a player,
 *          and a list of the loaded bodies (doesn't own bodies).
 */
render_info_t *render_column(scene_t *scene, list_t *level, size_t column,
                             double scroll_speed, vector_t absolute_origin);

/**
 * Frees render info from render_column
 */
void render_info_free(render_info_t *info);

#endif
