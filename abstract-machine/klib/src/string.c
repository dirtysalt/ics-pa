#include <klib-macros.h>
#include <klib.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char* s) {
    //  panic("Not implemented");
    size_t i = 0;
    while (s[i] != 0) i++;
    return i;
}

char* strcpy(char* dst, const char* src) {
    //    panic("Not implemented");
    size_t i = 0;
    while (src[i] != 0) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = 0;
    return dst;
}

char* strncpy(char* dst, const char* src, size_t n) {
    //    panic("Not implemented");
    size_t i = 0;
    for (i = 0; i < n && src[i]; i++) {
        dst[i] = src[i];
    }
    while (i < n) {
        dst[i] = 0;
        i += 1;
    }
    return dst;
}

char* strcat(char* dst, const char* src) {
    // panic("Not implemented");
    size_t n = strlen(dst);
    char* end = dst + n;
    size_t i = 0;
    for (i = 0; src[i]; i++) {
        end[i] = src[i];
    }
    end[i] = 0;
    return dst;
}

int strcmp(const char* s1, const char* s2) {
    // panic("Not implemented");
    size_t i = 0;
    while (s1[i] && s2[i]) {
        if (s1[i] < s2[i]) return -1;
        if (s1[i] > s2[i]) return 1;
        i += 1;
    }
    if (s1[i] == 0 && s2[i] == 0) return 0;
    if (s1[i] == 0) return -1;
    return 1;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    // panic("Not implemented");
    size_t i = 0;
    for (i = 0; i < n && s1[i] && s2[i]; i++) {
        if (s1[i] < s2[i]) return -1;
        if (s1[i] > s2[i]) return 1;
        i += 1;
    }
    if (i == n) return 0;
    if (s1[i] == 0 && s2[i] == 0) return 0;
    if (s1[i] == 0) return -1;
    return 1;
}

void* memset(void* s, int c, size_t n) {
    // panic("Not implemented");
    char* dst = (char*)s;
    for (size_t i = 0; i < n; i++) {
        dst[i] = (char)c;
    }
    return dst;
}

void* memmove(void* dst, const void* src, size_t n) {
    // panic("Not implemented");
    if (dst == src) return dst;
    char* b = (char*)dst;
    const char* a = (const char*)src;
    if (dst < src) {
        for (size_t i = 0; i < n; i++) {
            b[i] = a[i];
        }
    } else {
        for (size_t i = n - 1; i >= 1; i--) {
            b[i] = a[i];
        }
        b[0] = a[0];
    }
    return dst;
}

void* memcpy(void* out, const void* in, size_t n) {
    //   panic("Not implemented");
    char* dst = (char*)out;
    const char* src = (const char*)in;
    for (size_t i = 0; i < n; i++) {
        dst[i] = src[i];
    }
    return dst;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    // panic("Not implemented");
    const char* a = (const char*)s1;
    const char* b = (const char*)s2;
    for (size_t i = 0; i < n; i++) {
        if (a[i] < b[i]) return -1;
        if (a[i] > b[i]) return 1;
    }
    return 0;
}

#endif
