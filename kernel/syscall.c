#include <stdint.h>
#include <stddef.h>
#include "syscall.h"
#include "serial.h"
#include "console.h"
#include "task.h"
#include "uaccess.h"
#include "ipc.h"
#include "keyboard.h"

#define SYSCALL_WRITE_MAX 256U
#define SYSCALL_READ_MAX  128U

struct registers *syscall_dispatch(
    struct registers *regs
)
{
    if (regs == 0)
        return regs;

    switch (regs->eax) {

        case SYS_WRITE:
        {
            uint32_t user_buffer = regs->ebx;
            uint32_t length = regs->ecx;
            uint8_t buffer[SYSCALL_WRITE_MAX];

            if (length == 0U || length > SYSCALL_WRITE_MAX) {
                regs->eax = (uint32_t)-1;
                return regs;
            }

            if ((regs->cs & 0x3U) != 0x3U) {
                regs->eax = (uint32_t)-1;
                return regs;
            }

            if (!copy_from_user(buffer, (const void *)(uintptr_t)user_buffer, length)) {
                serial_write("[syscall] SYS_WRITE invalid user range\n");
                regs->eax = (uint32_t)-1;
                return regs;
            }

            for (uint32_t i = 0; i < length; ++i) {
                serial_putc((char)buffer[i]);
                console_putc((char)buffer[i]);
            }

            regs->eax = length;
            return regs;
        }

        case SYS_READ:
        {
            uint32_t user_buffer = regs->ebx;
            uint32_t length = regs->ecx;
            char buffer[SYSCALL_READ_MAX];

            if (length == 0U || length > SYSCALL_READ_MAX) {
                regs->eax = (uint32_t)-1;
                return regs;
            }

            if ((regs->cs & 0x3U) != 0x3U) {
                regs->eax = (uint32_t)-1;
                return regs;
            }

            if (!uaccess_verify_write((const void *)(uintptr_t)user_buffer, length)) {
                serial_write("[syscall] SYS_READ invalid user range\n");
                regs->eax = (uint32_t)-1;
                return regs;
            }

            __asm__ volatile ("sti");

            int count = keyboard_readline(buffer, length);
            if (count <= 0) {
                regs->eax = 0;
                return regs;
            }

            size_t copy_len = (size_t)count;
            if (copy_len + 1 <= length) {
                copy_len += 1;
            }

            if (!copy_to_user((void *)(uintptr_t)user_buffer, buffer, copy_len)) {
                serial_write("[syscall] SYS_READ copy_to_user failed\n");
                regs->eax = (uint32_t)-1;
                return regs;
            }

            regs->eax = (uint32_t)count;
            return regs;
        }

        case SYS_YIELD:

            if ((regs->cs & 0x3U) == 0x3U) {
                serial_write("[syscall] SYS_YIELD from Ring 3\n");
            }

            return task_scheduler_tick(regs);

        case SYS_EXIT:

            if ((regs->cs & 0x3U) != 0x3U) {
                regs->eax = (uint32_t)-1;
                return regs;
            }

            serial_write("[syscall] SYS_EXIT from Ring 3\n");
            task_exit();
            return task_scheduler_tick(regs);

        case SYS_SLEEP:
        {
            uint32_t ticks = regs->ebx;

            if ((regs->cs & 0x3U) != 0x3U) {
                regs->eax = (uint32_t)-1;
                return regs;
            }

            serial_write("[syscall] SYS_SLEEP from Ring 3\n");
            task_sleep(ticks);
            return regs;
        }

        case SYS_IPC_SEND:
        {
            uint32_t receiver_id = regs->ebx;
            uint32_t user_buffer = regs->ecx;
            uint32_t length = regs->edx;

            if ((regs->cs & 0x3U) != 0x3U) {
                regs->eax = (uint32_t)-1;
                return regs;
            }

            int result = ipc_send(receiver_id, (const void *)(uintptr_t)user_buffer, length);
            if (result <= 0) {
                serial_write("[syscall] SYS_IPC_SEND failed\n");
                regs->eax = (uint32_t)-1;
                return regs;
            }

            serial_write("[syscall] SYS_IPC_SEND ok\n");
            regs->eax = (uint32_t)result;
            return regs;
        }

        case SYS_IPC_RECV:
        {
            uint32_t user_buffer = regs->ebx;
            uint32_t capacity = regs->ecx;

            if ((regs->cs & 0x3U) != 0x3U) {
                regs->eax = (uint32_t)-1;
                return regs;
            }

            int result = ipc_receive((void *)(uintptr_t)user_buffer, capacity);
            if (result <= 0) {
                serial_write("[syscall] SYS_IPC_RECV empty/failed\n");
                regs->eax = (uint32_t)-1;
                return regs;
            }

            serial_write("[syscall] SYS_IPC_RECV ok\n");
            regs->eax = (uint32_t)result;
            return regs;
        }

        default:
            regs->eax = (uint32_t)-1;
            return regs;
    }
}
