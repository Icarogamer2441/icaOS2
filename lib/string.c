#include "string.h"
#include "stddef.h"

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

char* strcat(char* dest, const char* src) {
    char* d = dest;
    while (*d) d++;
    while ((*d++ = *src++));
    return dest;
}

size_t strlen(const char* str) {
    const char* s = str;
    while (*s) s++;
    return s - str;
}

char* strchr(const char* str, int ch) {
    while (*str && *str != ch) str++;
    return (*str == ch) ? (char*)str : NULL;
}

void* memcpy(void* dest, const void* src, size_t n) {
    char* d = dest;
    const char* s = src;
    while (n--) *d++ = *s++;
    return dest;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

char* strncpy(char* dest, const char* src, size_t n) {
    char* d = dest;
    while (n > 0 && *src) {
        *d++ = *src++;
        n--;
    }
    while (n > 0) {
        *d++ = '\0';
        n--;
    }
    return dest;
}

int snprintf(char* str, size_t size, const char* format, ...) {
    if (size == 0) return 0;
    
    size_t pos = 0;
    const char* ptr = format;
    
    while (*ptr && pos < size - 1) {
        if (*ptr == '%' && *(ptr + 1) == 's') {
            // Handle %s
            ptr += 2;
            const char* s = *(const char**)((&format) + 1);
            while (*s && pos < size - 1) {
                str[pos++] = *s++;
            }
        } else {
            str[pos++] = *ptr++;
        }
    }
    
    str[pos] = '\0';
    return pos;
}

void* memmove(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    if (d < s) {
        // Copia da esquerda para a direita
        while (n--) {
            *d++ = *s++;
        }
    } else if (d > s) {
        // Copia da direita para a esquerda
        d += n;
        s += n;
        while (n--) {
            *--d = *--s;
        }
    }
    
    return dest;
}

char* strrchr(const char* str, int ch) {
    char* last = NULL;
    while (*str) {
        if (*str == ch) {
            last = (char*)str;
        }
        str++;
    }
    return last;
}

int atoi(const char* str) {
    int result = 0;
    int sign = 1;
    
    // Pula espaços em branco
    while (*str == ' ') str++;
    
    // Verifica sinal
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    // Converte dígitos
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return sign * result;
}

float atof(const char* str) {
    float result = 0.0f;
    float fraction = 0.0f;
    float div = 1.0f;
    int sign = 1;
    
    // Pula espaços em branco
    while (*str == ' ') str++;
    
    // Verifica sinal
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    // Parte inteira
    while (*str >= '0' && *str <= '9') {
        result = result * 10.0f + (*str - '0');
        str++;
    }
    
    // Parte decimal
    if (*str == '.') {
        str++;
        while (*str >= '0' && *str <= '9') {
            div *= 10.0f;
            fraction = fraction * 10.0f + (*str - '0');
            str++;
        }
        result += fraction / div;
    }
    
    return sign * result;
} 