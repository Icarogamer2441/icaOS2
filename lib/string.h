#ifndef STRING_H
#define STRING_H

#include "stddef.h"

int strcmp(const char* s1, const char* s2);
char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);
size_t strlen(const char* str);
char* strchr(const char* str, int ch);
void* memcpy(void* dest, const void* src, size_t n);
int strncmp(const char* s1, const char* s2, size_t n);
char* strncpy(char* dest, const char* src, size_t n);
int snprintf(char* str, size_t size, const char* format, ...);
void* memmove(void* dest, const void* src, size_t n);
char* strrchr(const char* str, int ch);
int atoi(const char* str);
float atof(const char* str);

#endif 