#ifndef CTYPE_H
#define CTYPE_H

static inline int isalpha(int c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline int isdigit(int c) {
    return (c >= '0' && c <= '9');
}

static inline int isalnum(int c) {
    return isalpha(c) || isdigit(c);
}

#endif 