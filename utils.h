#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

extern const int ERROR;
extern const int INTERNAL_ERROR;

size_t get_size(FILE *f);

typedef struct {
    size_t size;
    uint8_t *arr;
} sized_char_array;

typedef struct {
    size_t size;
    uint32_t *arr;
} sized_int_array;
