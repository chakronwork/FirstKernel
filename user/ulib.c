#include "ulib.h"

int sys_write(const void *buf, uint32_t count)
{
    int ret;
    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "0"(1), "b"(buf), "c"(count)
        : "memory"
    );
    return ret;
}

int sys_read(void *buf, uint32_t count)
{
    int ret;
    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "0"(7), "b"(buf), "c"(count)
        : "memory"
    );
    return ret;
}

void sys_exit(int code)
{
    __asm__ volatile (
        "int $0x80"
        :
        : "a"(3), "b"(code)
        : "memory"
    );
    for (;;) {
    }
}

size_t strlen(const char *str)
{
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

void print(const char *str)
{
    if (!str)
        return;
    sys_write(str, (uint32_t)strlen(str));
}

int readline(char *buffer, uint32_t size)
{
    if (!buffer || size == 0)
        return 0;

    int n = sys_read(buffer, size - 1);
    if (n <= 0) {
        buffer[0] = '\0';
        return 0;
    }

    buffer[n] = '\0';

    while (n > 0 && (buffer[n - 1] == '\n' || buffer[n - 1] == '\r')) {
        buffer[n - 1] = '\0';
        n--;
    }

    return n;
}
