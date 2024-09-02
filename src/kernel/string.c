#include "xinos/string.h"

char *strcpy(char *dest, const char *src) {
    char *ptr = dest;
    while (true)
    {
        *ptr++ = *src;
        if(*src++ == EOS)
            return dest;
    }
    return NULL;
}

char *strcat(char *dest, const char *src) {
    char *ptr = dest;
    while(*ptr != EOS) ptr++;
    while (true)
    {
        *ptr++ = *src;
        if(*src++ == EOS)
            return dest;
    }
    return NULL;
}

size_t strlen(const char *str) {
    char *ptr = (char *)str;
    while(*ptr != EOS) {
        ptr++;
    }
    return ptr - str;
}

int strcmp(const char *lhs, const char *rhs) {
    char *p1 = (char*)lhs;
    char *p2 = (char*)rhs;
    while(true) {
        if(*p1 == EOS && *p2 == EOS) {
            return 0;
        }
        if(*p1 != *p2) {
            return *p1 < *p2 ? -1 : 1;
        }
        p1++;
        p2++;
    }
    return 0;
}

char *strchr(const char *str, int ch) {
    char *ptr = (char*)str;
    while(true) {
        if(*ptr == ch) {
            return ptr;
        }
        if(*ptr == EOS) {
            return NULL;
        }
        ptr++;
    }
    return NULL;
}

char *strrchr(const char *str, int ch) {
    char *ptr = (char*)str;
    char *ans = NULL;
    while(true) {
        if(*ptr == ch) {
            ans = ptr;
        }
        if(*ptr == EOS) {
            return ans;
        }
        ptr++;
    }
    return ans;
}

int memcmp(const void *lhs, const void *rhs, size_t count) {
    char *p1 = (char*)lhs;
    char *p2 = (char*)rhs;
    while(count > 0) {
        count--;
        if(*p1 == EOS && *p2 == EOS) {
            return 0;
        }
        if(*p1 != *p2) {
            return *p1 < *p2 ? -1 : 1;
        }
        p1++;
        p2++;
    }
    return 0;
}

void *memset(void *dest, int ch, size_t count) {
    char *ptr = (char*)dest;
    while(count--) {
        *ptr++ = ch;
    }
    return dest;
}

void *memcpy(void *dest, const void *src, size_t count) {
    char *ptr = (char*)dest;
    char *psrc = (char*)src;
    while(count--) {
        *ptr++ = *psrc;
        if(*psrc++ == EOS) {
            return dest;
        }
    }
    return dest;
}

void *memchr(const void *ptr, int ch, size_t count) {
    char *p = (char*)ptr;
    while(count--) {
        if(*p == ch) {
            return (void*)p;
        }
        if(*p == EOS) {
            return NULL;
        }
        p++;
    }
    return NULL;
}