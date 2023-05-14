#include "memory.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

void *allocated[10] = {NULL};
size_t last_allocated = 0;

void free_all(void) {
    for (size_t i = 0; i < last_allocated; i++) {
        free(allocated[i]);
        allocated[i] = NULL;
    }
    last_allocated = 0;
}
void *safe_malloc(size_t size) {
    void *p = malloc(size);
    if (p == NULL) {
        free_all();
        fprintf(stderr, "Couldn't allocate memory\n");
        exit(INTERNAL_ERROR);
    }
    allocated[last_allocated++] = p;
    return p;
}
FILE *safe_open(const char *file_name, const char *modes) {
    FILE *f = fopen(file_name, modes);
    if (f == NULL) {
        free_all();
        fprintf(stderr, "Error: file %s doesn't exist or can't be opened, creation failed\n", file_name);
        exit(ERROR);
    }
    return f;
}
void safe_close(FILE *f, const char *file_name) {
    if (fclose(f) == EOF) {
        free_all();
        fprintf(stderr, "Error: cannot close file %s\n", file_name);
        exit(INTERNAL_ERROR);
    }
}
