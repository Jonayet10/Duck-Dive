/** list.c
 *
 * Implementation for vex_list_t type defined in `list.h`.
 *
 * @author Gilbert Castro
 * @bug No known bugs
 */

#include "list.h"
#include <assert.h>
#include <stdlib.h>

int const GROWTH_FACTOR = 2;

struct list {
  void **data;
  size_t size;
  size_t capacity;
  free_func_t free_function;
};

list_t *list_init(size_t initial_size, free_func_t free_function) {
  list_t *list = malloc(sizeof(list_t));
  assert(list);
  list->data = malloc(initial_size * sizeof(void *));
  assert(list->data);
  list->size = 0;
  list->capacity = initial_size;
  list->free_function = free_function;
  return list;
}

void list_free(list_t *list) {
  if (list == NULL) {
    return;
  }
  if (list->free_function != NULL) {
    for (size_t i = 0; i < list->size; i++) {
      if (list->data[i] != NULL) {
        list->free_function(list->data[i]);
      }
    }
  }
  free(list->data);
  free(list);
}

size_t list_size(list_t *list) { return list->size; }

void *list_get(list_t *list, size_t index) {
  assert(index >= 0);
  assert(index < list->size);
  return list->data[index];
}

void ensure_capacity(list_t *list) {
  if (list->size + 1 > list->capacity) {
    if (list->capacity == 0) {
      list->capacity = 1;
    }
    list->capacity *= GROWTH_FACTOR;
    list->data = realloc(list->data, list->capacity * sizeof(void *));
  }
}

void list_add(list_t *list, void *value) {
  ensure_capacity(list);
  (list->data)[list->size] = value;
  list->size++;
}

void *list_remove(list_t *list, size_t index) {
  assert(index < list->size);
  assert(index >= 0);

  void *val = list->data[index];
  for (int i = index; i < list->size - 1; i++) {
    list->data[i] = list->data[i + 1];
  }

  list->size--;
  return val;
}
