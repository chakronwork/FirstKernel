.code32
.global _start

.text

_start:
    mov $1, %eax
    mov $welcome_msg, %ebx
    mov $(welcome_msg_end - welcome_msg), %ecx
    int $0x80

shell_loop:
    mov $1, %eax
    mov $prompt_msg, %ebx
    mov $(prompt_msg_end - prompt_msg), %ecx
    int $0x80

    sub $64, %esp

    mov $7, %eax
    mov %esp, %ebx
    mov $64, %ecx
    int $0x80

    cmp $0, %eax
    jle shell_next

    cmp $4, %eax
    jne not_help

    cmpl $0x706c6568, (%esp)
    jne not_help

    mov $1, %eax
    mov $help_msg, %ebx
    mov $(help_msg_end - help_msg), %ecx
    int $0x80
    jmp shell_next

not_help:
    mov $1, %eax
    mov $unknown_msg, %ebx
    mov $(unknown_msg_end - unknown_msg), %ecx
    int $0x80

shell_next:
    add $64, %esp
    jmp shell_loop

welcome_msg:
    .ascii "=== FirstOS User Shell (Ring 3) ===\n"
welcome_msg_end:

prompt_msg:
    .ascii "shell> "
prompt_msg_end:

help_msg:
    .ascii "Available commands:\n  help - show this message\n"
help_msg_end:

unknown_msg:
    .ascii "Unknown command. Type help for commands list.\n"
unknown_msg_end:
