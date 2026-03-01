#ifndef VEC_H
#define VEC_H

#define _POSIX_C_SOURCE 200809L

typedef enum {
    PLUS,
    MINUS,

    NEXT,
    PREV,

    PRINT,
    PRINT_NUMERICAL,
    PRINT_NUMERICAL_LN,
} Token;

typedef struct {
  Token *data;

  int len;
  int capacity;

  int element_size;
} TokenVector;

TokenVector make_vector(void);
void close_vector(TokenVector *);
void vector_push(TokenVector *, Token);
Token *vector_pop(TokenVector *);

#endif /* VEC_H */