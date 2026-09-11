#ifndef FIRSTOS_PAGE_FAULT_H
#define FIRSTOS_PAGE_FAULT_H

#include <stdint.h>
#include "idt.h"

struct registers *page_fault_handler(
    struct registers *regs
);

uint32_t page_fault_get_cr2(void);

#endif
