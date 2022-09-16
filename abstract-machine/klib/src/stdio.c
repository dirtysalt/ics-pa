#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static inline int write_str(char* s, int n, const char* buf, bool stdout) {
    if (stdout) {
        if (buf == NULL) {
            putstr("null");
            return 0;
        }
        for (int i = 0; buf[i]; i++) {
            putch(buf[i]);
        }
        return 0;
    }
    int i = 0;
    for (i = 0; i < n && buf[i]; i++) {
        s[i] = buf[i];
    }
    return i;
}

static inline int write_int(char* s, int n, int d, bool stdout) {
    char tmp[32];
    bool neg = false;
    if (d < 0) {
        d = -d;
        neg = true;
    }
    char* buf = tmp + 31;
    *buf = 0;
    if (d == 0) {
        *(--buf) = '0';
    }
    while (d != 0) {
        *(--buf) = '0' + d % 10;
        d = d / 10;
    }
    if (neg) {
        *(--buf) = '-';
    }
    return write_str(s, n, buf, stdout);
}

static inline int write_addr(char* s, int n, void* addr, bool stdout) {
    uintptr_t d = (uintptr_t)addr;
    char tmp[32];
    char* buf = tmp + 31;
    *buf = 0;
    if (d == 0) {
        *(--buf) = '0';
    }
    while (d != 0) {
        int v = d % 16;
        if (v >= 0 && v <= 9) {
            v += '0';
        } else {
            v += 'a' - 10;
        }
        *(--buf) = v;
        d = d / 16;
    }
    *(--buf) = 'x';
    *(--buf) = '0';
    return write_str(s, n, buf, stdout);
}

static inline int write_char(char* s, int n, char c, bool stdout) {
    if (stdout) {
        putch(c);
        return 0;
    }
    if (n == 0) {
        return 0;
    }
    *s = c;
    return 1;
}

static int _vsnprintf(char* out, size_t n, const char* fmt, bool stdout, va_list ap) {
    int j = 0;
    for (int i = 0; fmt[i] != 0; i++) {
        char c = fmt[i];
        if (c == '%') {
            // TODO(yan): I don't want to correctly handle alignment and prefix 0.
            while (fmt[i + 1] >= '0' && fmt[i + 1] <= '9') i++;
            c = fmt[i + 1];
            i++;
            if (c == 'd') {
                int val = va_arg(ap, int);
                j += write_int(out + j, n - j, val, stdout);
            } else if (c == 's') {
                const char* s = va_arg(ap, const char*);
                j += write_str(out + j, n - j, s, stdout);
            } else if (c == 'c') {
                char val = va_arg(ap, int);
                j += write_char(out + j, n - j, val, stdout);
            } else if (c == 'p') {
                void* val = va_arg(ap, void*);
                j += write_addr(out + j, n - j, val, stdout);
            } else {
                panic("directive not supported");
            }
        } else {
            j += write_char(out + j, n - j, c, stdout);
        }
        if (j >= n) return n;
    }
    if (!stdout) {
        out[j] = 0;
    }
    return j;
}

int printf(const char* fmt, ...) {
    va_list arglist;
    va_start(arglist, fmt);
    int ret = vsprintf(NULL, fmt, arglist);
    va_end(arglist);
    return ret;
}

int vsprintf(char* out, const char* fmt, va_list ap) {
    size_t n = 0x7fffffff;
    int ret = vsnprintf(out, n, fmt, ap);
    return ret;
}

int sprintf(char* out, const char* fmt, ...) {
    va_list arglist;
    va_start(arglist, fmt);
    int ret = vsprintf(out, fmt, arglist);
    va_end(arglist);
    return ret;
}

int snprintf(char* out, size_t n, const char* fmt, ...) {
    // panic("Not implemented");
    va_list arglist;
    va_start(arglist, fmt);
    int ret = vsnprintf(out, n, fmt, arglist);
    va_end(arglist);
    return ret;
}

int vsnprintf(char* out, size_t n, const char* fmt, va_list ap) {
    bool stdout = (out == NULL) ? true : false;
    return _vsnprintf(out, n, fmt, stdout, ap);
}
#endif
