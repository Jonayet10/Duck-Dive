#include "collision.h"
#include "polygon.h"
#include "scene.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  double min;
  double max;
} proj_extrema_t;

// Function Prototypes
collision_info_t find_collision_helper(list_t *shape1, list_t *shape2);
proj_extrema_t project_vertices(list_t *vertices, vector_t axis);
double vec_length(vector_t vec);
vector_t vec_normalize(vector_t vec);

collision_info_t find_collision(list_t *shape1, list_t *shape2) {
  collision_info_t one = find_collision_helper(shape1, shape2);
  collision_info_t two = find_collision_helper(shape2, shape1);
  vector_t axis = VEC_ZERO;
  bool collided = false;
  if (one.da_overlap < two.da_overlap) {
    axis = one.axis;
  } else {
    axis = two.axis;
  }
  collided = !(one.collided == false || two.collided == false);
  collision_info_t returned = {collided, axis, 0};
  return returned;
}

collision_info_t find_collision_helper(list_t *shape1, list_t *shape2) {
  double da_overlap = INFINITY;
  vector_t da_axis = VEC_ZERO;
  int collision_check = 1;

  for (size_t i = 0; i < list_size(shape1); i++) { // for each edge
    vector_t *v1 = list_get(shape1, i);
    vector_t *v2 = list_get(shape1, (i + 1) % list_size(shape1));

    vector_t edge = vec_subtract(*v2, *v1);
    vector_t axis = {edge.y, -edge.x};

    proj_extrema_t poly1 = project_vertices(shape1, axis);
    proj_extrema_t poly2 = project_vertices(shape2, axis);

    if (poly1.min >= poly2.max || poly2.min >= poly1.max) {
      collision_check = 0; // axis of separation exists, no collision
      break;               // should break out of the for loop
    }
    double overlap = poly2.max - poly1.min;
    if (poly1.max - poly2.min < overlap) {
      overlap = poly1.max - poly2.min;
    }
    if (overlap < da_overlap) {
      da_overlap = overlap;
      da_axis = axis;
    }
  }

  da_overlap /= vec_length(da_axis); // to get the actual overlap
  da_axis = vec_normalize(da_axis);

  // to ensure that da_axis is in the correct direction, point from shape1 to
  // shape2
  vector_t shape1_centroid = polygon_centroid(shape1);
  vector_t shape2_centroid = polygon_centroid(shape2);
  vector_t direction = vec_subtract(shape2_centroid, shape1_centroid);

  if (vec_dot(direction, da_axis) < 0) {
    da_axis = vec_negate(da_axis);
  }

  collision_info_t returned = {collision_check == 1 ? true : false, da_axis,
                               da_overlap};
  return returned;
}

/**
 * Projects the vectors in vertices to the axis.
 * @param vertices the vertices
 * @param axis the axis to project onto (axis is perpendicular to an edge of
 * polygon)
 * @return {min, max} the minimum and maximum projection as a vector_t
 */
proj_extrema_t project_vertices(list_t *vertices, vector_t axis) {
  double min = INFINITY;
  double max = -INFINITY;

  for (size_t i = 0; i < list_size(vertices); i++) {
    vector_t *vertex = list_get(vertices, i);
    double projection = vec_dot(*vertex, axis);

    if (projection < min) {
      min = projection;
    }
    if (projection > max) {
      max = projection;
    }
  }
  return (proj_extrema_t){min, max};
}

double vec_length(vector_t vec) { return sqrt(pow(vec.x, 2) + pow(vec.y, 2)); }

vector_t vec_normalize(vector_t vec) {
  return (vector_t){vec.x / vec_length(vec), vec.y / vec_length(vec)};
}
