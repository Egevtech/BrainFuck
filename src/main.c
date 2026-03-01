#define _POSIX_C_SOURCE 200809L

/*
 * Важное замечание от автора
 * Сейчас программа работает нестабильно, непонятно и т.п.
 * Проект будет заброшен как минимум на эту неделю, потому вот так вот.
 * Починю потом, наверное
 *
 * не поминайте лихом
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "vec.h"

#define INPUT_FILENAME_LEN strlen(output_file_arg)

#ifndef BFSTD
#define BFSTD "build/libbfstd.a"
#endif

#define REG_TOKEN(symbol, token_name) \
  case symbol: \
    result = token_name; \
    return &result; \
    break

Token *parse_token(int sym) {
  static Token result;

  switch (sym) {
    REG_TOKEN('+', PLUS);
    REG_TOKEN('-', MINUS);

    REG_TOKEN('>', NEXT);
    REG_TOKEN('<', PREV);

    REG_TOKEN('.', PRINT);
    REG_TOKEN('c', PRINT);
    REG_TOKEN('C', PRINT_LN);

    REG_TOKEN('p', PRINT_NUMERICAL);
    REG_TOKEN('P', PRINT_NUMERICAL_LN);

  default:
    printf("Unexpected token: '%c'\n", sym);
    return NULL;
  }
}

int main(const int argc, char **argv) {
  printf("BFC - brainfuck compiler\n");

  char* output_file_arg = argv[2];

  if (argc != 3)
  {
    printf("USAGE: bfc [MODE] [FILE]\n"
            "[MODE] = {run, build}\n");
    return EXIT_FAILURE;
  }

  int need_run = 0;

  if (strcmp(argv[1], "run") == 0) {
      need_run = 1;
  } else if (strcmp(argv[1], "build") != 0)
  {
    // TODO: я переделаю это потом и сделаю нормальный обработчик
    printf("Invalid usage. Available modes are: run, build\n");
    return EXIT_FAILURE;
  }

  FILE *file = fopen(output_file_arg, "r");

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

  char *output_file = malloc(sizeof(char) * (strlen(output_file_arg) + 2));
  strcpy(output_file, output_file_arg);
  strcat(output_file, ".s");

  FILE *output = fopen(output_file, "w");
  free(output_file);

  if (output == NULL) {
    printf("Can't open output file\n");
    return EXIT_FAILURE;
  }

  const char *externs = "vector_init, next_cell, prev_cell, add_cell, "
                        "sub_cell, print_cell, print_cell_ln, print_cell_num, print_cell_num_ln";

  fprintf(
      output,
      "section .data\nglobal _start\n\nvec dq 0\n"
      "cell dd 0\n\nextern %s\n\nsection .text\n_start:\n\tcall vector_init\n"
      "\tmov [vec], rax\n\n",
      externs);

  char *operation = malloc(20 * sizeof(char));

  for (int i = 0; i < tokens.len; i++) {
    int pr = ((float)i + 1.00f) / tokens.len * 100.0f;

    printf("Compiling... %d%%\r", pr);

    #define TOKEN2CALL(token, call) \
      case token: \
        strcpy(operation, #call); \
        break

    switch (tokens.data[i]) {
      TOKEN2CALL(PLUS, add_cell);
      TOKEN2CALL(MINUS, sub_cell);
      TOKEN2CALL(NEXT, next_cell);
      TOKEN2CALL(PREV, prev_cell);
      TOKEN2CALL(PRINT, print_cell);
      TOKEN2CALL(PRINT_LN, print_cell_ln);
      TOKEN2CALL(PRINT_NUMERICAL, print_cell_num);
      TOKEN2CALL(PRINT_NUMERICAL_LN, print_cell_num_ln);
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
  const char *link_cmd_pattern = "ld %s.o %s -o %s.out -lc "
                                 "-dynamic-linker /lib64/ld-linux-x86-64.so.2";

  char *compile_cmd = malloc(strlen(compile_cmd_pattern) + strlen(output_file_arg) * 2 + 1);
  char *link_cmd = malloc(strlen(link_cmd_pattern) +
    strlen(output_file_arg) * 2 + 5 + strlen(BFSTD));

  FILE* stdlib_file = fopen(BFSTD, "r");
  char* fstdlib_name = malloc(sizeof(char) * 30);

  if (stdlib_file == NULL) {
    strcpy(fstdlib_name, "/usr/local/lib/libbfstd.a");
    free(stdlib_file);
  } else {
    strcpy(fstdlib_name, BFSTD);
    fclose(stdlib_file);
  }

  sprintf(compile_cmd, compile_cmd_pattern, output_file_arg, output_file_arg);
  sprintf(link_cmd, link_cmd_pattern, output_file_arg, fstdlib_name, output_file_arg);

  free(fstdlib_name);

  printf("Building obj file... ");

  FILE *build_cmd_pipe = popen(compile_cmd, "r");
  if (build_cmd_pipe == NULL) {
    printf("fail\nError opening pipe for building\n");
    free(compile_cmd);
    free(link_cmd);
    return EXIT_FAILURE;
  }

  int *build_result = malloc(sizeof(int));
  if ((*build_result = pclose(build_cmd_pipe)) != 0) {
    printf("fail\nBuild failed with the exit code %d\n", *build_result);
    free(compile_cmd);
    free(link_cmd);
    free(build_result);
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

  sprintf(nasm_file, "%s.s", output_file_arg);
  sprintf(object_file, "%s.o", output_file_arg);

  if (remove(nasm_file) != 0 || remove(object_file) != 0) {
    free(nasm_file);
    free(object_file);
    printf("fail\n");
    perror("Cleaning failed");
    return EXIT_FAILURE;
  }

  free(nasm_file);
  free(object_file);

  printf("done\n");

  if (need_run != 1) {
      return EXIT_SUCCESS;
  }

  printf("Preparing for run...");

  char* output_filename = malloc(sizeof(char) * INPUT_FILENAME_LEN + 4);

  sprintf(output_filename, "%s.out", output_file_arg);


  if (access(output_filename, F_OK)) {
      printf(" fail\nCan't find executable file %s\n", output_filename);

      free(output_filename);
      return EXIT_FAILURE;
  }

  printf("done\n");

  printf("Running %s\n", output_filename);

  execv(output_filename, NULL); //TODO: Link file into output_file_arg.out across a.out

  free(output_filename);

  return EXIT_SUCCESS;
}
