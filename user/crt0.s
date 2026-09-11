.code32
.global _start

.text

_start:
    call main

    mov %eax, %ebx
    mov $3, %eax
    int $0x80

1:
    jmp 1b
