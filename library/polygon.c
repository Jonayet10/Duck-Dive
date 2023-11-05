#include "polygon.h"
#include <stdio.h>
#include <stdlib.h>

double polygon_area(list_t *polygon) {

  double current_area = 0.0;
  size_t size = list_size(polygon);

  for (size_t i = 0; i < size; i++) {
    current_area += vec_cross(*(vector_t *)list_get(polygon, i % size),
                              *(vector_t *)list_get(polygon, (i + 1) % size));
  }

  return current_area / 2;
}

vector_t polygon_centroid(list_t *polygon) {

  vector_t centroid = VEC_ZERO;
  size_t size = list_size(polygon);

  if (size == 1) {
    return *(vector_t *)list_get(polygon, 0);
  }

  for (size_t i = 0; i < size; i++) {
    vector_t v1 = *(vector_t *)list_get(polygon, i % size);
    vector_t v2 = *(vector_t *)list_get(polygon, (i + 1) % size);
    vector_t temp =
        vec_add(centroid, vec_multiply(vec_cross(v1, v2), vec_add(v1, v2)));
    centroid.x = temp.x;
    centroid.y = temp.y;
  }
  return vec_multiply(1 / (6 * polygon_area(polygon)), centroid);
}

void polygon_translate(list_t *polygon, vector_t translation) {
  size_t polygon_size = list_size(polygon);
  for (size_t i = 0; i < polygon_size; i++) {
    *(vector_t *)list_get(polygon, i) =
        vec_add(*(vector_t *)list_get(polygon, i), translation);
  }
}

void polygon_rotate(list_t *polygon, double angle, vector_t point) {
  polygon_translate(polygon, vec_negate(point));

  size_t polygon_size = list_size(polygon);
  for (size_t i = 0; i < polygon_size; i++) {
    *(vector_t *)list_get(polygon, i) =
        vec_rotate(*(vector_t *)list_get(polygon, i), angle);
  }
  polygon_translate(polygon, point);
}
