#include <stdlib.h>
#include <stdio.h>

#include "vec.h"

TokenVector make_vector(void) {
  const TokenVector tv = {
      .data = malloc(sizeof(Token)),
      .len = 0,
      .capacity = sizeof(Token),
      .element_size = sizeof(Token),
  };

  return tv;
}

void close_vector(TokenVector *vec) {
  free(vec->data);
  vec->len = 0;
  vec->capacity = 0;
}

// Return NULL against exit the program
void vector_push(TokenVector *vec, const Token token) {
  if (vec->capacity < vec->len * vec->element_size) {
    printf("Abnormally vector size!\n");

    close_vector(vec);
    exit(EXIT_FAILURE);
  }

  vec->len++;

  if (vec->capacity < vec->len * vec->element_size) {
    vec->data = (Token*)realloc(vec->data, vec->len*vec->element_size);
    vec->capacity = vec->len * vec->element_size;
  }

  vec->data[vec->len - 1] = token;
}

Token *vector_pop(TokenVector *vec) {
  if (vec->len == 0) {
    printf("Trying to pop an empty vector!\n");
    return NULL;
  }

  vec->len--;

  printf("New len: %d\n", vec->len);
  return &vec->data[vec->len - 1];
}