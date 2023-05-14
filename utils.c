#include "utils.h"
#include <stdio.h>

const int ERROR = 1;
const int INTERNAL_ERROR = 2;

size_t get_size(FILE *f) {
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    return size;
}