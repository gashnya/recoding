#pragma once
#include <stdio.h>
#include <stdlib.h>

extern void *allocated[10];
extern size_t last_allocated;

void free_all(void);

void *safe_malloc(size_t size);

FILE *safe_open(const char *file_name, const char *modes);

void safe_close(FILE *f, const char *file_name);
