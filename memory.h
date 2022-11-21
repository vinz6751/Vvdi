#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdbool.h>

bool mem_initialize(int32_t size, int n);
void *mem_allocate(int32_t size);
void mem_free(void *addr);

#endif

