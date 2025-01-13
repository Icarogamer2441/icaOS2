#include "memory.h"

#define HEAP_SIZE 1024*1024  // 1MB de heap
static unsigned char heap[HEAP_SIZE];
static size_t heap_ptr = 0;

void* malloc(size_t size) {
    if (heap_ptr + size > HEAP_SIZE) {
        return NULL;
    }
    
    void* ptr = &heap[heap_ptr];
    heap_ptr += size;
    return ptr;
}

void free(void* ptr) {
    // Por enquanto não implementamos free
    // Em um sistema real, precisaríamos gerenciar a memória adequadamente
    (void)ptr;
}

void* memset(void* dest, int val, unsigned int count) {
    unsigned char* ptr = (unsigned char*)dest;
    while (count-- > 0) {
        *ptr++ = val;
    }
    return dest;
} 