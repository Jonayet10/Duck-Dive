/** vector.c
 *
 * Implementation for a pass-by-value vector_t type defined in `vector.h`
 *
 *
 * @bug No known bugs
 */

#include "vector.h"
#include <math.h>
#include <stdio.h>

const vector_t VEC_ZERO = {.x = 0, .y = 0};

vector_t vec_add(vector_t v1, vector_t v2) {
  vector_t v = {.x = v1.x + v2.x, .y = v1.y + v2.y};
  return v;
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
  return vec_add(v1, vec_negate(v2));
}

vector_t vec_negate(vector_t v) { return vec_multiply(-1.0, v); }

vector_t vec_multiply(double scalar, vector_t v) {
  vector_t v_out = {.x = v.x * scalar, .y = v.y * scalar};
  return v_out;
}

double vec_dot(vector_t v1, vector_t v2) { return v1.x * v2.x + v1.y * v2.y; }

double vec_cross(vector_t v1, vector_t v2) { return v1.x * v2.y - v1.y * v2.x; }

vector_t vec_rotate(vector_t v, double angle) {
  vector_t v_out = {.x = v.x * cos(angle) - v.y * sin(angle),
                    v.x * sin(angle) + v.y * cos(angle)};
  return v_out;
}
