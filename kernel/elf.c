#include "elf.h"

#include <stdint.h>
#include <stddef.h>

#include "pmm.h"
#include "task.h"
#include "address_space.h"
#include "paging.h"

static void *kernel_memcpy(
    void *dest,
    const void *src,
    size_t n
)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    while (n--) {
        *d++ = *s++;
    }

    return dest;
}

static void *kernel_memset(
    void *dest,
    int value,
    size_t n
)
{
    uint8_t *d = (uint8_t *)dest;

    while (n--) {
        *d++ = (uint8_t)value;
    }

    return dest;
}

int elf_check_header(
    const elf32_ehdr_t *hdr
)
{
    if (hdr == 0)
        return -1;

    if (
        hdr->e_ident[0] != 0x7F ||
        hdr->e_ident[1] != 'E' ||
        hdr->e_ident[2] != 'L' ||
        hdr->e_ident[3] != 'F'
    ) {
        return -1;
    }

    if (hdr->e_ident[4] != 1)
        return -1;

    if (hdr->e_ident[5] != 1)
        return -1;

    if (hdr->e_type != ET_EXEC)
        return -1;

    if (hdr->e_machine != EM_386)
        return -1;

    if (hdr->e_version != 1)
        return -1;

    if (hdr->e_ehsize != sizeof(elf32_ehdr_t))
        return -1;

    if (hdr->e_phentsize != sizeof(elf32_phdr_t))
        return -1;

    return 0;
}

static int elf_map_segment(
    struct address_space *space,
    const elf32_phdr_t *ph,
    const uint8_t *data,
    size_t size
)
{
    if (space == 0 || ph == 0 || data == 0)
        return -1;

    if (ph->p_memsz < ph->p_filesz)
        return -1;

    if (
        (uint64_t)ph->p_offset +
        (uint64_t)ph->p_filesz >
        (uint64_t)size
    ) {
        return -1;
    }

    if (ph->p_memsz == 0)
        return 0;

    uint64_t segment_end =
        (uint64_t)ph->p_vaddr +
        (uint64_t)ph->p_memsz;

    if (segment_end > 0x100000000ULL)
        return -1;

    uint32_t start_page =
        ph->p_vaddr &
        ~(PAGE_SIZE - 1U);

    uint32_t end_page =
        (uint32_t)(
            (segment_end + PAGE_SIZE - 1ULL) &
            0xFFFFFFFFULL
        );

    if (
        end_page < start_page
    ) {
        return -1;
    }

    uint32_t flags =
        PAGE_PRESENT |
        PAGE_USER;

    if (
        (ph->p_flags & PF_W) != 0
    ) {
        flags |= PAGE_WRITABLE;
    }

    for (
        uint32_t page = start_page;
        page < end_page;
        page += PAGE_SIZE
    ) {
        uint32_t physical =
            pmm_alloc_page();

        if (physical == 0)
            return -1;

        if (
            !address_space_map_page(
                space,
                page,
                physical,
                flags
            )
        ) {
            pmm_free_page(
                physical
            );

            return -1;
        }

        uint8_t *page_memory =
            (uint8_t *)(uintptr_t)physical;

        kernel_memset(
            page_memory,
            0,
            PAGE_SIZE
        );
    }

    uint32_t page_offset =
        ph->p_vaddr &
        (PAGE_SIZE - 1U);

    uint32_t remaining =
        ph->p_filesz;

    const uint8_t *src =
        data + ph->p_offset;

    uint32_t virtual_address =
        ph->p_vaddr;

    while (remaining > 0) {
        uint32_t page_virtual =
            virtual_address &
            ~(PAGE_SIZE - 1U);

        uint32_t offset =
            virtual_address &
            (PAGE_SIZE - 1U);

        uint32_t chunk =
            PAGE_SIZE - offset;

        if (chunk > remaining)
            chunk = remaining;

        uint32_t physical = 0;
        uint32_t mapped_flags = 0;

        if (
            !address_space_get_page(
                space,
                page_virtual,
                &physical,
                &mapped_flags
            )
        ) {
            return -1;
        }

        uint8_t *destination =
            (uint8_t *)(uintptr_t)physical;

        kernel_memcpy(
            destination + offset,
            src,
            chunk
        );

        virtual_address += chunk;
        src += chunk;
        remaining -= chunk;
    }

    (void)page_offset;

    return 0;
}

static int elf_setup_user_stack_for_task(
    struct task *task,
    uint32_t *stack_top_out
)
{
    if (task == 0)
        return -1;

    uint32_t stack_bottom =
        USER_STACK_TOP -
        USER_STACK_SIZE;

    for (
        uint32_t addr = stack_bottom;
        addr < USER_STACK_TOP;
        addr += PAGE_SIZE
    ) {
        uint32_t physical =
            pmm_alloc_page();

        if (physical == 0)
            return -1;

        if (
            !address_space_map_page(
                task->address_space,
                addr,
                physical,
                PAGE_PRESENT |
                PAGE_WRITABLE |
                PAGE_USER
            )
        ) {
            pmm_free_page(
                physical
            );

            return -1;
        }
    }

    if (stack_top_out != 0) {
        *stack_top_out =
            USER_STACK_TOP - 16U;
    }

    return 0;
}

int elf_setup_user_stack(
    uint32_t *stack_top_out
)
{
    if (stack_top_out == 0)
        return -1;

    *stack_top_out =
        USER_STACK_TOP - 16U;

    return 0;
}

int elf_load_executable(
    const void *data,
    size_t size,
    uint32_t *entry_out
)
{
    if (
        data == 0 ||
        size < sizeof(elf32_ehdr_t)
    ) {
        return -1;
    }

    const elf32_ehdr_t *hdr =
        (const elf32_ehdr_t *)data;

    if (
        elf_check_header(hdr) != 0
    ) {
        return -1;
    }

    uint64_t ph_end =
        (uint64_t)hdr->e_phoff +
        (
            (uint64_t)hdr->e_phnum *
            sizeof(elf32_phdr_t)
        );

    if (
        ph_end > (uint64_t)size
    ) {
        return -1;
    }

    if (entry_out != 0) {
        *entry_out =
            hdr->e_entry;
    }

    return 0;
}

int elf_spawn(
    const void *data,
    size_t size
)
{
    if (
        data == 0 ||
        size < sizeof(elf32_ehdr_t)
    ) {
        return -1;
    }

    const elf32_ehdr_t *hdr =
        (const elf32_ehdr_t *)data;

    if (
        elf_check_header(hdr) != 0
    ) {
        return -1;
    }

    uint64_t ph_end =
        (uint64_t)hdr->e_phoff +
        (
            (uint64_t)hdr->e_phnum *
            sizeof(elf32_phdr_t)
        );

    if (
        ph_end > (uint64_t)size
    ) {
        return -1;
    }

    uint32_t entry =
        hdr->e_entry;

    uint32_t stack_top =
        USER_STACK_TOP - 16U;

    uint32_t task_id =
        task_create_user(
            entry,
            stack_top
        );

    if (task_id == 0)
        return -1;

    struct task *task =
        task_get(task_id);

    if (task == 0)
        return -1;

    task->state =
        TASK_BLOCKED;

    for (
        uint16_t i = 0;
        i < hdr->e_phnum;
        ++i
    ) {
        const elf32_phdr_t *ph =
            (
                const elf32_phdr_t *
            )(
                (const uint8_t *)data +
                hdr->e_phoff +
                (
                    (uint32_t)i *
                    sizeof(elf32_phdr_t)
                )
            );

        if (
            ph->p_type != PT_LOAD
        ) {
            continue;
        }

        if (
            elf_map_segment(
                task->address_space,
                ph,
                (const uint8_t *)data,
                size
            ) != 0
        ) {
            return -1;
        }
    }

    if (
        elf_setup_user_stack_for_task(
            task,
            &stack_top
        ) != 0
    ) {
        return -1;
    }

    task->user_entry =
        entry;

    task->user_esp =
        stack_top;

    task->user_mode =
        1;

    task->state =
        TASK_READY;

    return (int)task_id;
}
