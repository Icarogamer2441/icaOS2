#ifndef MEMORY_H
#define MEMORY_H

#include "stddef.h"

void* malloc(size_t size);
void free(void* ptr);
void* memcpy(void* dest, const void* src, unsigned int count);
void* memset(void* dest, int val, unsigned int count);

#endif 