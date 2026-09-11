#ifndef ULIB_H
#define ULIB_H

#include <stdint.h>
#include <stddef.h>

int sys_write(const void *buf, uint32_t count);
int sys_read(void *buf, uint32_t count);
void sys_exit(int code);

size_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);
void print(const char *str);
int readline(char *buffer, uint32_t size);

#endif
