#ifndef __BLOCK_H__
#define __BLOCK_H__

#include "body.h"
#include "list.h"
#include <stdlib.h>

extern const double BLOCK_WIDTH;

typedef struct obstacle_info obstacle_info_t;

/**
 * Initializes a new block and returns the body
 *
 * @param body_type the type of the block to be created
 * @return body_t* the body created from the initialization
 */
body_t *block_init(body_type_t body_type);

/**
 * Sets the grid position of the block
 *
 * @param block the block to be moved
 * @param x_index the x index on the grid (0 on the left)
 * @param y_index the y index on the grid (0 on the bottom)
 * @param absolute_origin the actual location of the 0,0 block after scrolling
 */
void block_set_pos(body_t *block, size_t x_index, size_t y_index,
                   vector_t absolute_origin);

/**
 * Gets the time since an action occurred with the block
 *
 * @param block the block to get the time since
 * @return time since an action occurred
 */
double block_get_time_since(body_t *block);

/**
 * Gets the time between actions for the block
 *
 * @param block the block to get time between actions
 * @return the constant time between actions
 */
double block_get_time(body_t *block);

/**
 * Sets the time since an action occurred with the block
 *
 * @param block the block to get the time since
 * @param time amount of time to set
 */
void block_set_time_since(body_t *block, double time);

/**
 * Gets the list of projectiles associated with the block
 *
 * @param block the block to find projectiles from
 * @return list of projectiles
 */
list_t *block_get_projs(body_t *block);

/**
 * Adds a projectile to the list of projectiles associated with the block
 *
 * @param block the block to add projectiles to
 * @param projectile the projectile to add to the list
 */
void block_add_proj(body_t *block, body_t *projectile);
#endif
