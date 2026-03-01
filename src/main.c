#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vec.h"

#define INPUT_FILENAME_LEN sizeof(argv[1])

#ifndef BFSTD
#define BFSTD "./libbfstd.a"
#endif

#define RET_TOKEN(token_name)                                                  \
  result = token_name;                                                         \
  return &result;                                                              \
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
    printf("Unexpected token: '%c'\n", sym);
    return NULL;
  }
}

int main(const int argc, char **argv) {
  printf("BFC - brainfuck compiler\n");

  if (argc != 2) {
    printf("USAGE: bfc FILE\n"
           "USAGE: FILE - file to compile\n");
    return EXIT_FAILURE;
  }

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


  fclose(file);

  void *output_file = malloc(sizeof(char) * (strlen(argv[1]) + 2));
  strcpy(output_file, argv[1]);
  strcat(output_file, ".s");

  FILE *output = fopen(output_file, "w");

  if (output == NULL) {
    printf("Can't open output file\n");
    free(output_file);
    return EXIT_FAILURE;
  }

  const char *externs = "vector_init, next_cell, prev_cell, add_cell, "
                        "sub_cell, print_cell, print_num, print_num_ln";

  fprintf(
      output,
      "section .data\nglobal _start\n\nvec dq 0\n"
      "cell dd 0\n\nextern %s\n\nsection .text\n_start:\n\tcall vector_init\n"
      "\tmov [vec], rax\n\n",
      externs);

  char *operation = malloc(20 * sizeof(char));

  for (int i = 0; i < tokens.len; i++) {
    int pr = ((float)i + 1) / tokens.len * 100.0f;

    printf("Compiling... %d%%\r", pr);

    switch (tokens.data[i]) {
    case PLUS:
      strcpy(operation, "add_cell");
      break;
    case MINUS:
      strcpy(operation, "sub_cell");
      break;

    case NEXT:
      strcpy(operation, "next_cell");
      break;
    case PREV:
      strcpy(operation, "prev_cell");
      break;

    case PRINT:
      strcpy(operation, "print_cell");
      break;
    case PRINT_NUMERICAL:
      strcpy(operation, "print_num");
      break;
    case PRINT_NUMERICAL_LN:
      strcpy(operation, "print_num_ln");
      break;
    }

    fprintf(output, "\tmov rdi, [vec]\n\tmov rsi, cell\n\tcall %s\n\n",
            operation);
  }

  close_vector(&tokens);

  free(operation);

  fprintf(output, "\tmov rax, 60\n\tmov rdi, 0\n\tsyscall");
  fclose(output);

  printf("Compiling... done\n");

  const char *compile_cmd_pattern = "nasm -felf64 %s.s -o %s.o";
  const char *link_cmd_pattern = "ld %s.o %s -o a.out -lc "
                                 "-dynamic-linker /lib64/ld-linux-x86-64.so.2";

  char *compile_cmd = malloc(strlen(compile_cmd_pattern) + strlen(argv[1]) * 2 + 1);
  char *link_cmd = malloc(strlen(link_cmd_pattern) + 
    strlen(argv[1]) * 2 + 1 + strlen(BFSTD));

  FILE* stdlib_file = fopen(BFSTD, "r");
  char* fstdlib_name = malloc(sizeof(char) * 30);

  if (stdlib_file == NULL) {
    strcpy(fstdlib_name, "/usr/local/lib/libbfstd.a");
    free(stdlib_file);
  } else {
    strcpy(fstdlib_name, BFSTD);
    fclose(stdlib_file);
  }

  sprintf(compile_cmd, compile_cmd_pattern, argv[1], argv[1]);
  sprintf(link_cmd, link_cmd_pattern, argv[1], fstdlib_name, argv[1]);

  free(fstdlib_name);

  printf("Building obj file... ");

  FILE *build_cmd_pipe = popen(compile_cmd, "r");
  if (build_cmd_pipe == NULL) {
    printf("fail\nError opening pipe for building\n");
    free(compile_cmd);
    free(link_cmd);
    free(output_file);
    return EXIT_FAILURE;
  }

  int *build_result = malloc(sizeof(int));
  if ((*build_result = pclose(build_cmd_pipe)) != 0) {
    printf("fail\nBuild failed with the exit code %d\n", *build_result);
    free(compile_cmd);
    free(link_cmd);
    free(build_result);
    free(output_file);
    return EXIT_FAILURE;
  }

  free(build_result);
  free(compile_cmd);
  printf("done\n");

  printf("Linking a.out... ");

  FILE *link_cmd_pipe = popen(link_cmd, "r");
  if (link_cmd_pipe == NULL) {
    printf("fail\nError opening pipe for linking\n");
    free(link_cmd);
    free(output_file);
    return EXIT_FAILURE;
  }

  int *link_result = malloc(sizeof(int));
  if ((*link_result = pclose(link_cmd_pipe)) != 0) {
    printf("fail\nLink failed with the exit code %d\n", *link_result);

    free(link_cmd);
    free(output_file);
    free(link_result);
    return EXIT_FAILURE;
  }

  printf("done\n");

  free(link_cmd);
  free(link_result);
  free(output_file);

  printf("Cleaning... ");

  char* nasm_file = malloc(sizeof(char) * ( 4 + INPUT_FILENAME_LEN));
  char* object_file = malloc(sizeof(char) * ( 6 + INPUT_FILENAME_LEN));
  if (nasm_file == NULL | object_file == NULL) {
    printf("fail\n");
    perror("Allocation failed");

    if (nasm_file == NULL) free(nasm_file);
    if (object_file == NULL) free(object_file);

    return EXIT_FAILURE;
  }

  sprintf(nasm_file, "./%s.s", argv[1]);
  sprintf(object_file, "./%s.o", argv[1]);

  if (remove(nasm_file) != 0 || remove(object_file) != 0) {
    free(nasm_file);
    free(object_file);
    printf("fail\n");
    perror("Cleaning failed");
  }

  free(nasm_file);
  free(object_file);

  printf("done\n");
  
  return EXIT_SUCCESS;
}
