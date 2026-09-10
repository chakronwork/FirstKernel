#ifndef ELF_H
#define ELF_H

#include <stddef.h>
#include <stdint.h>

#define ELF_MAGIC 0x464C457F

#define ET_NONE 0
#define ET_REL  1
#define ET_EXEC 2

#define EM_386  3

#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6

#define PF_X 1
#define PF_W 2
#define PF_R 4

#define USER_STACK_TOP  0xBFFFF000
#define USER_STACK_SIZE 0x4000

typedef struct {
    uint8_t  e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} __attribute__((packed)) elf32_ehdr_t;

typedef struct {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} __attribute__((packed)) elf32_phdr_t;

int elf_check_header(const elf32_ehdr_t *hdr);
int elf_load_executable(const void *data, size_t size, uint32_t *entry_out);
int elf_setup_user_stack(uint32_t *stack_top_out);
int elf_spawn(const void *data, size_t size);

#endif
