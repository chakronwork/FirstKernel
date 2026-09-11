#include "uaccess.h"

bool is_user_range(const void *ptr, size_t size) {
    uintptr_t start = (uintptr_t)ptr;
    uintptr_t end = start + size;

    if (size == 0) {
        return true;
    }

    if (end < start) {
        return false;
    }

    if (start >= USER_SPACE_LIMIT || end > USER_SPACE_LIMIT) {
        return false;
    }

    return true;
}

bool uaccess_verify_read(const void *ptr, size_t size) {
    return is_user_range(ptr, size);
}

bool uaccess_verify_write(const void *ptr, size_t size) {
    return is_user_range(ptr, size);
}

int copy_from_user(void *dest, const void *src, size_t size) {
    if (!uaccess_verify_read(src, size)) {
        return 0;
    }

    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    for (size_t i = 0; i < size; i++) {
        d[i] = s[i];
    }

    return 1;
}

int copy_to_user(void *dest, const void *src, size_t size) {
    if (!uaccess_verify_write(dest, size)) {
        return 0;
    }

    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    for (size_t i = 0; i < size; i++) {
        d[i] = s[i];
    }

    return 1;
}
