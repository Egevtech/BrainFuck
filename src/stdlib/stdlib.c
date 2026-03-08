#include <stdlib.h>
#include <stdio.h>

struct DataVector
{
    size_t capacity;
    int* data;

    int len;
};

struct DataVector *vector_init(void)
{
    struct DataVector *ret_vector = malloc(sizeof(struct DataVector));

    if (ret_vector == NULL) {
        printf("Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    *ret_vector = (struct DataVector) {
        .data = malloc(sizeof(int)),
        .len = 1,
        .capacity = sizeof(int),
    };

    return ret_vector;
}

void vector_close(const struct DataVector *vector) {
    free(vector->data);
}

void vector_push(struct DataVector *vec, int push_data)
{
    if ( vec->capacity < vec->len * sizeof(int))
    {
        printf("\nAbnormal vector size!\n");
        exit(EXIT_FAILURE);
    }

    vec->len++;

    if ( vec->capacity < vec->len * sizeof(int))
    {
        vec->data = realloc(vec->data, vec->len * sizeof(int));
        vec->capacity = vec->len * sizeof(int);
    }

    vec->data[vec->len-1] = push_data;
}

int vector_get(const struct DataVector* vec, const int dest) {
    return vec->data[dest];
}

void vector_set(const struct DataVector *vec, const int dest, const int new_data)
{
    if (dest >= vec->len || dest < 0)
    {
        printf("Vector is too small!\n");
        exit(EXIT_FAILURE);
    }
    vec->data[dest] = new_data;
}

void next_cell(struct DataVector* vec, int *current_cell)
{
    // printf("next_cell\n");
    if ( vec == NULL || current_cell == NULL )
    {
        printf("Get NULL, when expected vec or current_cell\n");
        exit(EXIT_FAILURE);
    }

    (*current_cell) += 1;

    if (*(current_cell) >= vec->len)
    {
        vector_push(vec, 0);
    }

}

void prev_cell(struct DataVector *, int *current_cell)
{
    if (*current_cell == 0) {
        printf("No space to move left!\n");

        exit(EXIT_FAILURE);
    }

    *current_cell -= 1;
}

void add_cell(struct DataVector* vec, const int *current_cell) {
    vector_set(vec, *current_cell, vector_get(vec, *current_cell) + 1);
}

void sub_cell(struct DataVector *vec, const int *current_cell) {
    vector_set(vec, *current_cell, vector_get(vec, *current_cell) - 1);
}

void print_cell(struct DataVector *vec, int *current_cell) {
    printf("%c", vector_get(vec, *current_cell));
}

void print_cell_ln(struct DataVector *vec, int *current_cell) {
    print_cell(vec, current_cell);
    printf("\n");
}

void print_cell_num(struct DataVector *vec, int *current_cell) {
    printf("%d", vector_get(vec, *current_cell));
}

void print_cell_num_ln(struct DataVector *vec, int *current_cell) {
    print_cell_num(vec, current_cell);
    printf("\n");
}