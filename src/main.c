#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <threads.h>

// TODO: Move Token operations and vector from this file
typedef enum {
  PLUS,
  MINUS,

  NEXT,
  PREV,

  PRINT,
  PRINT_NUMERICAL,
  PRINT_NUMERICAL_LN,
} Token;

#define RET_TOKEN(token_name) \
  result = token_name;        \
  return &result; \
  break

Token *parse_token(int sym) {
  static Token result;

  switch (sym) {
    case '+':
      RET_TOKEN(PLUS);

    case '-':
      RET_TOKEN(MINUS);

    case '>':
      RET_TOKEN(NEXT);

    case '<':
      RET_TOKEN(PREV);

    case '.':
      RET_TOKEN(PRINT);

    case 'p':
      RET_TOKEN(PRINT_NUMERICAL);

    case 'P':
      RET_TOKEN(PRINT_NUMERICAL_LN);

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
    vec->data = reallocarray(vec->data, vec->len, vec->element_size);
    vec->capacity = vec->len * vec->element_size;
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

  int symbol;
  int char_count = 0;
  TokenVector tokens = make_vector();

  while ((symbol = fgetc(file)) != EOF) {
    if (isspace(symbol))
      continue;
    const Token *token = parse_token(symbol);
    char_count++;

    if (token == NULL) {
      printf("Error at symbol %d\n", char_count);

      close_vector(&tokens);
      return EXIT_FAILURE;
    }

    vector_push(&tokens, *token);
  }

  printf("Tokens parsed: %d\n", tokens.len);

  fclose(file);

  void* output_file = malloc(sizeof(char) * (strlen(argv[1]) + 2));
  strcpy(output_file, argv[1]);
  strcat(output_file, ".s");

  printf("Opening output file %s\n", (char*)output_file);
  FILE* output = fopen(output_file, "w");

  if (output == nullptr) {
    printf("Can't open output file\n");
    free(output_file);
    return EXIT_FAILURE;
  }

  const char* externs = "vector_init, next_cell, prev_cell, add_cell, sub_cell, print_cell, print_num, print_num_ln";

  fprintf(output, "section .data\nglobal _start\n\nvec dq 0\n"
    "cell dd 0\n\nextern %s\n\nsection .text\n_start:\n\tcall vector_init\n"
    "\tmov [vec], rax\n\n", externs);

  char* operation = malloc(20 * sizeof(char));

  for ( int i = 0; i < tokens.len; i++ )
  {
    int pr = ((float)i + 1) / tokens.len * 100.0f;

    printf("Compiling... %d%%\r", pr);

    switch ( tokens.data[i] )
    {
    case PLUS:
      strcpy(operation,"add_cell");
      break;
    case MINUS:
      strcpy(operation,"sub_cell");
      break;

    case NEXT:
      strcpy(operation,"next_cell");
      break;
    case PREV:
      strcpy(operation,"prev_cell");
      break;

    case PRINT:
      strcpy(operation,"print_cell");
      break;
    case PRINT_NUMERICAL:
      strcpy(operation,"print_num");
      break;
    case PRINT_NUMERICAL_LN:
      strcpy(operation,"print_num_ln");
      break;
    }

    fprintf(output, "\tmov rdi, [vec]\n\tmov rsi, cell\n\tcall %s\n\n", operation);

  }

  free(operation);


  fprintf(output, "\tmov rax, 60\n\tmov rdi, 0\n\tsyscall");
  fclose(output);

  close_vector(&tokens); // TODO: Check if this is on its place

  printf("Compiling... done\n");

  const char *compile_cmd_pattern = "nasm -felf64 %s.s -o %s.o";
  const char *link_cmd_pattern = "ld %s.o build/libstdbf.a -o a.out -lc -dynamic-linker /lib64/ld-linux-x86-64.so.2";

  char* compile_cmd = malloc(strlen(compile_cmd_pattern) + strlen(argv[1]) * 2 + 1);
  char* link_cmd = malloc(strlen(link_cmd_pattern) + strlen(argv[1]) * 2 + 1);

  sprintf(compile_cmd, compile_cmd_pattern, argv[1], argv[1]);
  sprintf(link_cmd, link_cmd_pattern, argv[1], argv[1]);

  printf("Building... ");

  FILE* build_cmd_pipe = popen(compile_cmd, "r");
  if ( build_cmd_pipe == nullptr) {
    printf("fail\nError opening pipe for building\n");
    free(compile_cmd);
    free(link_cmd);
    free(output_file);
    return EXIT_FAILURE;
  }

  int *build_result = malloc(sizeof(int));
  if ((*build_result = pclose(build_cmd_pipe)) != 0)
  {
    printf("fail\nBuild failed with the exit code %d", *build_result);
    free(compile_cmd);
    free(link_cmd);
    free(build_result);
    free(output_file);
    return EXIT_FAILURE;
  }
  free(build_result);
  free(compile_cmd);
  printf("done\n");

  printf("Linking... ");

  FILE* link_cmd_pipe = popen(link_cmd, "r");
  if ( link_cmd_pipe == nullptr)
  {
    printf("fail\nError opening pipe for linking\n");
    free(link_cmd);
    free(output_file);
    return EXIT_FAILURE;
  }

  int *link_result = malloc(sizeof(int));
  if ((*link_result = pclose(link_cmd_pipe)) != 0) {
    printf("fail\nLink failed with the exit code %d", *link_result);

    free(link_cmd);
    free(output_file);
    free(link_result);
    return EXIT_FAILURE;
  }

  printf("done\n");

  free(link_cmd);
  free(link_result);
  free(output_file);
  return EXIT_SUCCESS;
}
