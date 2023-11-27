/**
 * @brief Definition of ADT dynamic pushdownay.
 * @author Jakub Kloub, xkloub03, FIT VUT
 * @author Matúš Moravčík, xmorav48, FIT VUT
 * @date 10/11/2023
 */
#include "pushdown.h"
#include <stdlib.h>
#include "error.h"

bool pushdown_should_resize(size_t size, size_t capacity) {
  return size == capacity;
}

bool handle_resize(void** p_data, size_t sizeof_item, size_t size, size_t* capacity) {
  if (pushdown_should_resize(size, *capacity)) {
    *capacity = *capacity * 2;
    *p_data = realloc(*p_data, *capacity * sizeof_item);
    if (!(*p_data)) {
      SET_INT_ERROR(IntError_Memory, "handle_resize(): Realloc failed for `p_data`.");
      return false;
    }
  }

  return true;
}

void pushdown_init(Pushdown* pushdown) {
  pushdown->capacity = DEFAULT_CAPACITY;
  pushdown->size = 0;
  pushdown->data = malloc(DEFAULT_CAPACITY * sizeof(PushdownItem));
  if (!pushdown->data)
    set_error(Error_Internal);
}

void pushdown_destroy(Pushdown* pushdown) {
  free(pushdown->data);
  pushdown->data = ((void*)0);
  pushdown->size = pushdown->capacity = 0;
}

void pushdown_push_back(Pushdown* pushdown, PushdownItem* value) {
  if (!handle_resize((void**)&pushdown->data, sizeof(Pushdown), pushdown->size, &pushdown->capacity))
    return;
  pushdown->data[pushdown->size++] = *value;
}

void pushdown_insert(Pushdown* pushdown, size_t index, PushdownItem* value) {
  if (index > pushdown->size) {
    set_error(Error_Internal);
    return;
  }
  if (!handle_resize((void**)&pushdown->data, sizeof(Pushdown), pushdown->size, &pushdown->capacity))
    return;
  for (size_t i = pushdown->size; i > index; i--) {
    pushdown->data[i] = pushdown->data[i - 1];
  }
  pushdown->size++;
  pushdown->data[index] = *value;
}

PushdownItem pushdown_back(const Pushdown* pushdown) {
  return pushdown->data[pushdown->size - 1];
}

PushdownItem pushdown_at(Pushdown* pushdown, size_t idx) {
  return pushdown->data[idx];
}

void pushdown_reduce_size(Pushdown* pushdown, size_t new_size) {
  if (new_size > pushdown->size) {
    set_error(Error_Internal);
    return;
  }
  pushdown->size = new_size;
}

PushdownItem* create_pushdown_item(Token* term, struct NTerm* nterm) {
  PushdownItem* item = malloc(sizeof(PushdownItem));
  item->nterm = nterm;
  item->terminal = term;
  item->name = '|';  // default name: end of rule
  return item;
}

PushdownItem* set_name(PushdownItem* item, char name) {
  item->name = name;
  return item;
}
