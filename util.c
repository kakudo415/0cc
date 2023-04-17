#include "0cc.h"

void *new_vec() {
  Vec *vec = malloc(sizeof(Vec));
  vec->ptr = malloc(sizeof(void *) * 8);
  vec->cap = 8;
  vec->len = 0;
  return vec;
}

void vec_push(Vec *vec, void *elm) {
  if (vec->cap <= vec->len) {
    vec->cap *= 2;
    vec->ptr = realloc(vec->ptr, sizeof(void *) * vec->cap);
  }
  vec->ptr[(vec->len)++] = elm;
}

void *vec_pop(Vec *vec) {
  if (vec->len == 0) {
    return NULL;
  }
  return vec->ptr[--(vec->len)];
}
