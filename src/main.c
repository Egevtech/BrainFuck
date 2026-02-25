#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
  PLUS,
  MINUS,

  NEXT,

  PRINT,
} Token;

#define RET_TOKEN(token_name) \
  printf(" %s\n", #token_name);  \
  result = token_name;        \
  return &result; \
  break

Token *parse_token(int sym) {
  static Token result;

  printf("Symbol %c parsed as", sym);

  switch (sym) {
    case '+':
      RET_TOKEN(PLUS);

    case '-':
      RET_TOKEN(MINUS);

    case '>':
      RET_TOKEN(NEXT);

    case '.':
      RET_TOKEN(PRINT);

    default:
      printf("... nothing?\nUnexpected token: '%c'\n", sym);
      return nullptr;
  }
}

typedef struct {
  Token* data;

  int len;
  int capacity;

  int element_size;
} TokenVector;

TokenVector make_vector(void) {
  constexpr size_t t = sizeof(Token);

  const TokenVector tv = {
    .data = malloc(t),
    .len = 0,
    .capacity = sizeof(Token),
    .element_size = sizeof(Token),
  };

  return tv;
}

void close_vector(TokenVector* vec)
{
  free(vec->data);
  vec->len = 0;
  vec->capacity = 0;
  printf("Vector closed\n");
}

// TODO: if error, return nullptr, don't escape program here
void vector_push(TokenVector* vec, const Token token)
{

  // printf("%d * %d (%d) = %d (needs %d)\n", vec->len, vec->element_size, vec->len * vec->element_size, vec->capacity, (vec->len + 1) * vec->element_size);
  if ( vec->capacity < vec->len * vec->element_size ) {
    printf("Abnormally vector size!\n");

    close_vector(vec);
    exit(EXIT_FAILURE);
  }

  vec->len++;

  if ( vec->capacity < vec->len * vec->element_size ) {
    printf("Trying to realloc array with new size: %d (old is %d)\n", vec->len * vec->element_size, (vec->len - 1) * vec->element_size);
    vec->data = reallocarray(vec->data, vec->len, vec->element_size);
    vec->capacity = vec->len * vec->element_size;
  } else {
    printf("No reallocation needed\n");
  }

  vec->data[vec->len - 1] = token;
}

Token *vector_pop(TokenVector* vec) {
  if ( vec->len == 0 ) {
    printf("Trying to pop an empty vector!\n");
    return nullptr;
  }

  vec->len--;

  printf("New len: %d\n", vec->len);
  return &vec->data[vec->len - 1];
}

int main(const int argc, char **argv)
{
  printf("BFC - brainfuck compiler\n");

  if (argc != 2) {
    printf("USAGE: bfc FILE\n"
           "USAGE: FILE - file to compile\n");
    return EXIT_FAILURE;
  }

  printf("Reading file %s\n", argv[1]);

  FILE *file = fopen(argv[1], "r");

  if (file == NULL) {
    printf("Open file failed\n");
    return EXIT_FAILURE;
  }

  int t;
  int char_count = 0;
  TokenVector tokens = make_vector();

  while ((t = fgetc(file)) != EOF) {
    if (isspace(t))
      continue;
    const Token *token = parse_token(t);
    char_count++;

    if (token == NULL) {
      printf("Error at symbol %d\n", char_count);

      close_vector(&tokens);
      return EXIT_FAILURE;
    }

    vector_push(&tokens, *token);
  }

  printf("Tokens parsed: %d\n", tokens.len);

  for ( int i = 0 ; i < tokens.len ; i++ )
  {
    printf("%d, ", tokens.data[i]);
  }
  printf("\n");

  fclose(file);

  close_vector(&tokens); // TODO: Check if this is on its place
  return EXIT_SUCCESS;
}
