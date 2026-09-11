#ifndef UACCESS_H
#define UACCESS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define USER_SPACE_LIMIT 0xC0000000

bool is_user_range(const void *ptr, size_t size);
bool uaccess_verify_read(const void *ptr, size_t size);
bool uaccess_verify_write(const void *ptr, size_t size);
int copy_from_user(void *dest, const void *src, size_t size);
int copy_to_user(void *dest, const void *src, size_t size);

#endif
