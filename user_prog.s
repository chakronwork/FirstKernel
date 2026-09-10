.code32
.global _start

.text

_start:
    mov $1, %eax
    mov $msg, %ebx
    mov $(msg_end - msg), %ecx
    int $0x80

    mov $3, %eax
    int $0x80

1:
    jmp 1b

msg:
    .ascii "Welcome to firstOS!\n"
msg_end:
