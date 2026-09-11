# firstOS

A small 32-bit x86 operating system kernel developed as a low-level systems programming and operating systems research project.

firstOS is designed to explore the fundamental components of an operating system, including bootstrapping, protected mode, interrupt handling, physical and virtual memory management, task scheduling, user-mode execution, system calls, inter-process communication, executable loading, and basic user-space interaction.

The project is intentionally small and self-contained so that the relationship between hardware, kernel code, memory, privilege levels, and user-space programs remains visible.

> **Project status:** Active development
> **Target architecture:** 32-bit x86
> **Execution environment:** QEMU / Multiboot-compatible boot environments
> **Kernel language:** C and x86 assembly
> **License:** MIT

---

# ภาษาไทย

## ภาพรวม

firstOS เป็นโครงการพัฒนาเคอร์เนลระบบปฏิบัติการขนาดเล็กสำหรับสถาปัตยกรรม x86 แบบ 32 บิต โดยมีวัตถุประสงค์หลักเพื่อศึกษาการทำงานของระบบปฏิบัติการตั้งแต่ระดับการบูตเครื่อง ไปจนถึงการจัดการหน่วยความจำ การจัดตารางงาน การทำงานใน Ring 3 และการเรียกใช้บริการของเคอร์เนลผ่าน system call

โครงการนี้ไม่ได้มีเป้าหมายเป็นระบบปฏิบัติการสำหรับใช้งานทั่วไป แต่เน้นการสร้างส่วนประกอบของ kernel ขึ้นมาทีละส่วน พร้อมทดสอบพฤติกรรมของแต่ละระบบในสภาพแวดล้อมที่ควบคุมได้

---

## เป้าหมายของโครงการ

เป้าหมายหลักของ firstOS คือการสร้างระบบปฏิบัติการขนาดเล็กที่สามารถแสดงลำดับการทำงานตั้งแต่:

```text
Boot
  |
  v
Multiboot
  |
  v
Kernel Initialization
  |
  +--> GDT
  |
  +--> IDT
  |
  +--> PIC / PIT
  |
  +--> Physical Memory Manager
  |
  +--> Paging / Virtual Memory
  |
  +--> Address Spaces
  |
  +--> Task Scheduler
  |
  +--> User Mode / Ring 3
  |
  +--> System Calls
  |
  +--> IPC
  |
  +--> User Program / Shell
```

แนวทางนี้ช่วยให้โครงสร้างของ operating system สามารถศึกษาได้ตั้งแต่ระดับ hardware interface ไปจนถึง user-space execution

---

## ความสามารถปัจจุบัน

ปัจจุบัน repository มีส่วนประกอบหลักดังต่อไปนี้

### Boot and Kernel Initialization

* Multiboot 1 kernel entry
* 32-bit x86 protected mode
* Kernel linker script
* Kernel entry point
* Multiboot information parsing
* Multiboot module support
* Initrd registration

### CPU and Interrupt Management

* Global Descriptor Table (GDT)
* Interrupt Descriptor Table (IDT)
* Programmable Interrupt Controller (PIC)
* Programmable Interval Timer (PIT)
* CPU exception handling
* Hardware interrupt handling
* IRQ-based scheduling

### Console and Debugging

* VGA text-mode console
* VGA hardware cursor
* Serial console output
* Hexadecimal and decimal diagnostic output
* Kernel boot diagnostics

### Physical Memory

* Physical Memory Manager (PMM)
* Physical page allocation
* Physical page release
* Multiboot memory information
* Kernel heap allocator
* Kernel heap validation
* Heap stress testing

### Virtual Memory

* Paging
* Page tables
* Virtual Memory Manager (VMM)
* Page fault handling
* Address space abstraction
* Per-task address spaces
* User-accessible page mappings

### Task Management

* Static task table
* Task creation
* Task states:

  * `UNUSED`
  * `READY`
  * `RUNNING`
  * `BLOCKED`
  * `DEAD`
* Task switching
* Preemptive scheduling
* PIT-driven scheduler ticks
* Task sleeping
* Task waking
* Idle task
* User task support

The current task subsystem supports up to 32 task slots and maintains dedicated task context and user-mode execution state.

### User Mode and Privilege Separation

firstOS supports execution of user programs in Ring 3.

The kernel maintains separate execution information for user tasks, including:

* user entry address
* user stack
* kernel stack
* user address space
* user code mapping
* user-owned physical pages

This allows the kernel to distinguish kernel-mode execution from user-mode execution.

### System Calls

System calls are dispatched through the kernel syscall layer.

Current syscall identifiers are:

| Number | Name           | Purpose                           |
| ------ | -------------- | --------------------------------- |
| 1      | `SYS_WRITE`    | Write data from user space        |
| 2      | `SYS_YIELD`    | Yield the CPU                     |
| 3      | `SYS_EXIT`     | Terminate the current user task   |
| 4      | `SYS_SLEEP`    | Sleep for a number of timer ticks |
| 5      | `SYS_IPC_SEND` | Send an IPC message               |
| 6      | `SYS_IPC_RECV` | Receive an IPC message            |
| 7      | `SYS_READ`     | Read input into user memory       |

These interfaces are implemented in `kernel/syscall.c` and declared in `include/syscall.h`.

### User Memory Access

The kernel includes explicit user-memory validation mechanisms for system calls.

For example, `SYS_WRITE` validates user-space buffers before copying data into kernel space, while `SYS_READ` validates writable user memory before copying input back to the user process.

This separation is important because a kernel must not blindly trust pointers supplied by Ring 3 code.

### Inter-Process Communication

The kernel provides an initial IPC interface through:

```text
SYS_IPC_SEND
SYS_IPC_RECV
```

IPC is integrated into the syscall layer and is intended to provide communication between tasks and future user-space processes.

### ELF Support

The repository includes an initial ELF32 executable loader.

The loader currently validates:

* ELF magic
* 32-bit class
* little-endian format
* executable file type
* x86 architecture
* ELF version
* ELF header size
* program-header size

It also contains functionality for mapping loadable ELF segments into a task address space and creating a user stack.

The project therefore has the foundation for executing externally prepared user-space ELF programs rather than embedding every user program directly into kernel source code.

### Initrd

The first Multiboot module is treated as the initrd image.

The kernel obtains:

```text
module start
module end
module size
```

from the Multiboot information structure and registers the module before starting user-space execution.

The Makefile currently builds `user_prog.elf` and copies it to `initrd.img`, which is then passed to GRUB or QEMU as a Multiboot module.

---

# สถาปัตยกรรมโดยรวม

โครงสร้างของ firstOS แบ่งออกเป็นชั้นหลักดังนี้

```text
Hardware
    |
    v
Boot / Multiboot
    |
    v
CPU Initialization
    |
    +-- GDT
    +-- IDT
    +-- PIC
    +-- PIT
    |
    v
Memory Subsystem
    |
    +-- PMM
    +-- Paging
    +-- VMM
    +-- Address Space
    +-- Kernel Heap
    |
    v
Task Subsystem
    |
    +-- Task Creation
    +-- Context Switching
    +-- Scheduler
    +-- Sleep / Wake
    |
    v
Protection Boundary
    |
    +-- Ring 0
    |
    +-- Ring 3
    |
    v
System Call Layer
    |
    +-- SYS_WRITE
    +-- SYS_READ
    +-- SYS_YIELD
    +-- SYS_SLEEP
    +-- SYS_EXIT
    +-- SYS_IPC_SEND
    +-- SYS_IPC_RECV
    |
    v
User Space
    |
    +-- User Program
    +-- User Shell
    +-- ELF Executable
```

---

# โครงสร้าง repository

โครงสร้างหลักของ repository ในปัจจุบันประกอบด้วย:

```text
Firstkernel/
├── boot/
│   └── boot.s
│
├── include/
│   ├── address_space.h
│   ├── console.h
│   ├── elf.h
│   ├── gdt.h
│   ├── idt.h
│   ├── initrd.h
│   ├── ipc.h
│   ├── keyboard.h
│   ├── kmalloc.h
│   ├── multiboot.h
│   ├── page_fault.h
│   ├── paging.h
│   ├── pic.h
│   ├── pmm.h
│   ├── serial.h
│   ├── syscall.h
│   ├── task.h
│   ├── timer.h
│   ├── tss.h
│   ├── uaccess.h
│   ├── user_test.h
│   ├── vga.h
│   └── vmm.h
│
├── kernel/
│   ├── address_space.c
│   ├── console.c
│   ├── elf.c
│   ├── gdt.c
│   ├── gdt.s
│   ├── idt.c
│   ├── idt.s
│   ├── initrd.c
│   ├── ipc.c
│   ├── keyboard.c
│   ├── kmalloc.c
│   ├── kmain.c
│   ├── page_fault.c
│   ├── paging.c
│   ├── pic.c
│   ├── pmm.c
│   ├── serial.c
│   ├── syscall.c
│   ├── task.c
│   ├── task_asm.s
│   ├── timer.c
│   ├── tss.c
│   ├── uaccess.c
│   ├── user_mode.s
│   ├── user_test.c
│   ├── vga.c
│   └── vmm.c
│
├── user_prog.s
├── user.ld
├── linker.ld
├── Makefile
├── LICENSE
├── SECURITY.md
└── README.md
```

---

# Boot Flow

ลำดับการเริ่มต้นระบบโดยสรุป:

1. Multiboot-compatible loader โหลด kernel
2. `boot/boot.s` เริ่มต้นการทำงาน
3. Kernel stack ถูกจัดเตรียม
4. เข้าสู่ `kmain`
5. ตรวจสอบ Multiboot magic
6. เริ่มต้น serial console
7. เริ่มต้น VGA console
8. อ่าน Multiboot modules
9. ลงทะเบียน initrd
10. โหลด GDT
11. โหลด IDT
12. ตั้งค่า PIC
13. ตั้งค่า PIT
14. เริ่มระบบ keyboard
15. เริ่ม PMM
16. เริ่ม kernel heap
17. เริ่ม paging / virtual memory
18. เริ่ม task subsystem
19. สร้าง kernel tasks
20. เตรียม user-space tasks
21. เปิดใช้งาน interrupt
22. scheduler เริ่มทำงาน
23. user-space สามารถเข้าสู่ Ring 3 และเรียก system calls

---

# Memory Model

firstOS ใช้โครงสร้างหน่วยความจำหลายระดับ:

```text
Physical Memory
      |
      v
Physical Memory Manager
      |
      v
Page Frames
      |
      v
Paging
      |
      v
Virtual Address Space
      |
      +------------------+
      |                  |
      v                  v
   Kernel Space       User Space
                         |
                         v
                    User Process
```

Kernel linker script กำหนดให้ kernel image เริ่มต้นที่ address `1 MiB` และแยก section หลัก เช่น `.text`, `.rodata`, `.data` และ `.bss` อย่างชัดเจน

---

# Task and Scheduler Model

Task subsystem ใช้ task table แบบคงที่และรองรับ task สูงสุด 32 ตัวใน configuration ปัจจุบัน

สถานะของ task:

```text
UNUSED
READY
RUNNING
BLOCKED
DEAD
```

Scheduler ใช้ PIT timer interrupt เป็นส่วนหนึ่งของกลไก preemption ทำให้ task สามารถถูกสลับออกจาก CPU ได้แม้ task นั้นจะไม่ได้เรียก `yield()` ด้วยตัวเอง

ระบบยังมี:

* sleeping task
* wake-up tick
* idle task
* kernel task
* user-mode task
* task-specific address space

---

# User Program

user-space program ปัจจุบันถูกสร้างจาก `user_prog.s`

ตัวอย่างการทำงานพื้นฐาน:

```text
User Program
    |
    +-- SYS_WRITE
    |
    +-- SYS_READ
    |
    +-- Parse input
    |
    +-- help
    |
    +-- unknown command
```

ตัวโปรแกรมมี shell loop พื้นฐานและรองรับคำสั่ง `help` ในระดับต้น โดยเรียกบริการของ kernel ผ่าน `int 0x80` แทนการเข้าถึง kernel โดยตรง

---

# Building

## Requirements

สภาพแวดล้อมสำหรับ build โดยทั่วไปต้องมี:

* GCC
* GNU binutils / linker
* QEMU
* GRUB tools สำหรับการสร้าง ISO
* ระบบ Linux ที่รองรับ 32-bit compilation

Makefile ปัจจุบันใช้:

```text
-m32
-ffreestanding
-fno-stack-protector
-fno-pie
-mno-red-zone
```

และ link kernel แบบ freestanding โดยไม่พึ่งพา libc

---

# Build Kernel

```bash
make
```

ผลลัพธ์หลัก:

```text
firstos.bin
```

---

# Run with QEMU

```bash
make run
```

คำสั่งนี้จะ build dependency ที่จำเป็นสำหรับ `initrd.img` และเรียก:

```bash
qemu-system-i386 -kernel firstos.bin -initrd initrd.img -serial stdio
```

ตามที่กำหนดไว้ใน Makefile ปัจจุบัน

---

# Build ISO

```bash
make iso
```

คำสั่งนี้จะ:

1. build kernel
2. build user program
3. สร้าง `initrd.img`
4. สร้าง GRUB configuration
5. สร้าง bootable ISO ด้วย `grub-mkrescue`

ผลลัพธ์:

```text
firstos.iso
```

---

# Clean

```bash
make clean
```

ลบ build artifacts รวมถึง:

```text
*.o
firstos.bin
firstos.iso
user_prog.elf
initrd.img
iso/
```

---

# Debugging

firstOS มี serial output สำหรับตรวจสอบสถานะของ kernel ระหว่าง boot และ runtime

ตัวอย่างข้อมูลที่ kernel สามารถรายงาน:

```text
[ ok ] multiboot verified
[ ok ] kernel console ready
[ ok ] initrd module loaded
[info] initrd start: ...
[info] initrd size: ...
```

ระบบ serial และ VGA console ถูกใช้งานคู่กันเพื่อให้ง่ายต่อการตรวจสอบปัญหาที่เกิดขึ้นก่อนที่ระบบ user-space จะเริ่มทำงาน

---

# Current Development Status

firstOS อยู่ในระหว่างการพัฒนาและยังไม่ควรถูกพิจารณาว่าเป็นระบบปฏิบัติการสำหรับ production

องค์ประกอบที่มีอยู่ใน repository แล้วครอบคลุมตั้งแต่ bootloader interface ไปจนถึง user-space execution แต่หลาย subsystem ยังอยู่ในขั้น experimental และยังจำเป็นต้องมีการทดสอบเพิ่มเติมในด้าน correctness, isolation, resource cleanup และ robustness

---

# Known Limitations

โครงการยังมีข้อจำกัดสำคัญ เช่น:

* รองรับเฉพาะ x86 32-bit
* ไม่ใช่ระบบปฏิบัติการสำหรับใช้งานทั่วไป
* driver support ยังมีจำกัด
* filesystem ยังไม่ใช่ระบบเต็มรูปแบบ
* user-space API ยังอยู่ในช่วงเริ่มต้น
* IPC ยังอยู่ในขั้นพื้นฐาน
* ELF loader ยังอยู่ในระหว่างการพัฒนา
* memory management ยังไม่มีคุณสมบัติระดับ production
* hardware compatibility ยังจำกัด
* security model ยังอยู่ในช่วงพัฒนา

ดังนั้นการรันบน hardware จริงควรถือเป็นขั้นทดลอง ไม่ใช่การใช้งานจริง

---

# Roadmap

แนวทางการพัฒนาต่อที่เหมาะสมกับโครงสร้างปัจจุบัน ได้แก่:

## Kernel

* ปรับปรุง memory management
* ปรับปรุง page fault handling
* ปรับปรุง process/task lifecycle
* ปรับปรุง resource cleanup
* เพิ่ม kernel synchronization primitives
* เพิ่ม timer abstraction
* เพิ่ม driver architecture

## User Space

* ปรับปรุง ELF loader
* เพิ่ม executable lifecycle
* เพิ่ม process isolation
* เพิ่ม standard user-space runtime
* เพิ่ม shell command system
* แยก user programs ออกจาก kernel test code

## System Calls

* ออกแบบ syscall ABI ให้มีเสถียรภาพมากขึ้น
* เพิ่ม process management
* เพิ่ม file abstraction
* เพิ่ม memory mapping
* เพิ่ม process identifiers
* เพิ่ม system-level error handling

## IPC

* ปรับปรุง message queues
* เพิ่ม synchronization
* เพิ่ม process-to-process communication model
* กำหนด IPC semantics ที่ชัดเจนขึ้น

## Storage

* เพิ่ม block device abstraction
* เพิ่ม filesystem
* รองรับ executable loading จาก storage
* พัฒนา virtual filesystem interface

---

# Design Principles

firstOS เน้นหลักการดังต่อไปนี้:

### 1. Small and Understandable

ระบบควรมีขนาดเล็กพอที่จะศึกษาความสัมพันธ์ระหว่าง component ได้ทั้งระบบ

### 2. Explicit Systems Programming

โค้ดควรแสดงให้เห็นการทำงานของ CPU, memory และ interrupt อย่างชัดเจน โดยลด abstraction ที่ไม่จำเป็น

### 3. Incremental Development

พัฒนาทีละ subsystem พร้อมทดสอบก่อนขยายความสามารถ

### 4. Kernel/User Separation

สิทธิ์ของ kernel และ user-space ควรถูกแยกออกจากกันตั้งแต่ระดับ execution privilege และ memory mapping

### 5. Testability

แต่ละ subsystem ควรมี diagnostic output หรือ test path ที่ช่วยตรวจสอบพฤติกรรมได้

---

# License

This project is licensed under the MIT License.

See [LICENSE](LICENSE) for the full license text.

---

# Security

Security-related information and the project's security policy are documented separately in [SECURITY.md](SECURITY.md).

---

# Repository

Source code:

https://github.com/chakronwork/Firstkernel

---

# English

## Overview

firstOS is a small 32-bit x86 operating system kernel developed as a low-level systems programming and operating systems research project.

The project focuses on understanding how an operating system is built from the bottom up, including bootstrapping, CPU initialization, interrupt handling, physical and virtual memory management, task scheduling, privilege separation, system calls, IPC, executable loading, and basic user-space interaction.

firstOS is not intended to be a production operating system. Its primary purpose is to provide a compact and understandable implementation of operating-system fundamentals.

---

## Project Goals

The project aims to demonstrate the progression from machine startup to user-space execution:

```text
Boot
  |
  v
Multiboot
  |
  v
Kernel Initialization
  |
  +--> GDT
  |
  +--> IDT
  |
  +--> PIC / PIT
  |
  +--> Physical Memory Manager
  |
  +--> Paging / Virtual Memory
  |
  +--> Address Spaces
  |
  +--> Task Scheduler
  |
  +--> User Mode / Ring 3
  |
  +--> System Calls
  |
  +--> IPC
  |
  +--> User Program / Shell
```

---

## Current Features

### Boot and Kernel Initialization

* Multiboot 1 kernel entry
* 32-bit x86 protected mode
* Custom kernel linker script
* Kernel entry point
* Multiboot information parsing
* Multiboot module support
* Initrd registration

### CPU and Interrupt Management

* Global Descriptor Table (GDT)
* Interrupt Descriptor Table (IDT)
* Programmable Interrupt Controller (PIC)
* Programmable Interval Timer (PIT)
* CPU exception handling
* Hardware interrupt handling
* IRQ-driven scheduling

### Console and Debugging

* VGA text-mode console
* VGA hardware cursor
* Serial console output
* Diagnostic hexadecimal output
* Diagnostic decimal output
* Boot and runtime status logging

### Physical Memory

* Physical Memory Manager (PMM)
* Physical page allocation
* Physical page release
* Multiboot memory information
* Kernel heap allocator
* Heap validation
* Heap stress testing

### Virtual Memory

* Paging
* Page tables
* Virtual Memory Manager (VMM)
* Page fault handling
* Address space abstraction
* Per-task address spaces
* User-space page mappings

### Task Management

* Static task table
* Task creation
* Task switching
* Preemptive scheduling
* PIT-driven scheduler ticks
* Task sleeping
* Task waking
* Idle task
* Kernel tasks
* User-mode tasks

The task subsystem currently provides up to 32 task slots and keeps task-specific kernel and user execution state.

### User Mode

firstOS supports user-mode execution in Ring 3.

User tasks maintain information including:

* user entry address
* user stack
* kernel stack
* address space
* user code mapping
* user-owned physical pages

This provides the foundation for privilege separation between kernel-mode and user-mode execution.

### System Calls

The current syscall interface contains:

| Number | Name           | Description                 |
| ------ | -------------- | --------------------------- |
| 1      | `SYS_WRITE`    | Write data from user space  |
| 2      | `SYS_YIELD`    | Yield the CPU               |
| 3      | `SYS_EXIT`     | Exit the current user task  |
| 4      | `SYS_SLEEP`    | Sleep for timer ticks       |
| 5      | `SYS_IPC_SEND` | Send an IPC message         |
| 6      | `SYS_IPC_RECV` | Receive an IPC message      |
| 7      | `SYS_READ`     | Read input into user memory |

The syscall implementation is located in `kernel/syscall.c`, with the ABI declarations in `include/syscall.h`.

### User Memory Validation

The syscall layer validates user-space pointers before copying data across the privilege boundary.

`SYS_WRITE` validates readable user memory before copying data to the kernel, while `SYS_READ` validates writable user memory before copying data back into user space.

### Inter-Process Communication

Initial IPC support is exposed through:

```text
SYS_IPC_SEND
SYS_IPC_RECV
```

This subsystem provides the initial foundation for communication between tasks and future processes.

### ELF Executable Support

The repository includes an ELF32 executable loader.

The loader validates:

* ELF magic
* 32-bit class
* little-endian encoding
* executable file type
* x86 architecture
* ELF version
* ELF header size
* program-header size

It also contains code for mapping loadable segments into a task address space and preparing a user-mode stack.

### Initrd

The first Multiboot module is used as the initrd image.

The kernel reads the module start and end addresses from the Multiboot information structure and registers the module before user-space initialization.

The current Makefile builds `user_prog.elf`, copies it to `initrd.img`, and provides the resulting image to GRUB or QEMU as a Multiboot module.

---

# Architecture

The major subsystems can be viewed as:

```text
Hardware
    |
    v
Boot / Multiboot
    |
    v
CPU Initialization
    |
    +-- GDT
    +-- IDT
    +-- PIC
    +-- PIT
    |
    v
Memory Subsystem
    |
    +-- PMM
    +-- Paging
    +-- VMM
    +-- Address Space
    +-- Kernel Heap
    |
    v
Task Subsystem
    |
    +-- Task Creation
    +-- Context Switching
    +-- Scheduler
    +-- Sleep / Wake
    |
    v
Protection Boundary
    |
    +-- Ring 0
    |
    +-- Ring 3
    |
    v
System Call Layer
    |
    +-- SYS_WRITE
    +-- SYS_READ
    +-- SYS_YIELD
    +-- SYS_SLEEP
    +-- SYS_EXIT
    +-- SYS_IPC_SEND
    +-- SYS_IPC_RECV
    |
    v
User Space
    |
    +-- User Program
    +-- User Shell
    +-- ELF Executable
```

---

# Repository Structure

```text
Firstkernel/
├── boot/
│   └── boot.s
│
├── include/
│   ├── address_space.h
│   ├── console.h
│   ├── elf.h
│   ├── gdt.h
│   ├── idt.h
│   ├── initrd.h
│   ├── ipc.h
│   ├── keyboard.h
│   ├── kmalloc.h
│   ├── multiboot.h
│   ├── page_fault.h
│   ├── paging.h
│   ├── pic.h
│   ├── pmm.h
│   ├── serial.h
│   ├── syscall.h
│   ├── task.h
│   ├── timer.h
│   ├── tss.h
│   ├── uaccess.h
│   ├── user_test.h
│   ├── vga.h
│   └── vmm.h
│
├── kernel/
│   ├── address_space.c
│   ├── console.c
│   ├── elf.c
│   ├── gdt.c
│   ├── gdt.s
│   ├── idt.c
│   ├── idt.s
│   ├── initrd.c
│   ├── ipc.c
│   ├── keyboard.c
│   ├── kmalloc.c
│   ├── kmain.c
│   ├── page_fault.c
│   ├── paging.c
│   ├── pic.c
│   ├── pmm.c
│   ├── serial.c
│   ├── syscall.c
│   ├── task.c
│   ├── task_asm.s
│   ├── timer.c
│   ├── tss.c
│   ├── uaccess.c
│   ├── user_mode.s
│   ├── user_test.c
│   ├── vga.c
│   └── vmm.c
│
├── user_prog.s
├── user.ld
├── linker.ld
├── Makefile
├── LICENSE
├── SECURITY.md
└── README.md
```

---

# Boot Flow

The initialization sequence can be summarized as:

1. A Multiboot-compatible loader loads the kernel.
2. `boot/boot.s` begins execution.
3. The kernel stack is prepared.
4. Control enters `kmain`.
5. The Multiboot magic value is validated.
6. The serial console is initialized.
7. The VGA console is initialized.
8. Multiboot modules are inspected.
9. The initrd is registered.
10. The GDT is loaded.
11. The IDT is loaded.
12. The PIC is configured.
13. The PIT is configured.
14. Keyboard support is initialized.
15. The physical memory manager is initialized.
16. The kernel heap is initialized.
17. Paging and virtual memory are initialized.
18. The task subsystem is initialized.
19. Kernel tasks are created.
20. User-space tasks are prepared.
21. Interrupts are enabled.
22. The scheduler starts running tasks.
23. User-space code can execute in Ring 3 and invoke system calls.

---

# Memory Model

The memory subsystem is structured approximately as:

```text
Physical Memory
      |
      v
Physical Memory Manager
      |
      v
Page Frames
      |
      v
Paging
      |
      v
Virtual Address Space
      |
      +------------------+
      |                  |
      v                  v
   Kernel Space       User Space
                         |
                         v
                    User Process
```

The kernel linker script places the kernel image at `1 MiB` and defines aligned `.text`, `.rodata`, `.data`, and `.bss` sections.

---

# Task and Scheduler Model

The scheduler uses a static task table with a current limit of 32 task slots.

Task states are:

```text
UNUSED
READY
RUNNING
BLOCKED
DEAD
```

The PIT timer interrupt provides the timing source for preemptive scheduling. This allows the scheduler to interrupt CPU-consuming tasks even when they do not voluntarily call `yield()`.

The task subsystem also supports:

* sleep
* wake-up scheduling
* idle task execution
* kernel tasks
* user tasks
* per-task address spaces

---

# User Program

The current user program is assembled from `user_prog.s`.

Its basic execution model is:

```text
User Program
    |
    +-- SYS_WRITE
    |
    +-- SYS_READ
    |
    +-- Parse input
    |
    +-- help
    |
    +-- unknown command
```

The program implements an initial Ring 3 shell loop and uses `int 0x80` to request kernel services rather than accessing kernel functionality directly.

---

# Building

## Requirements

Typical build requirements include:

* GCC
* GNU binutils / linker
* QEMU
* GRUB tools for ISO creation
* A Linux environment with 32-bit compilation support

The Makefile currently uses flags such as:

```text
-m32
-ffreestanding
-fno-stack-protector
-fno-pie
-mno-red-zone
```

and links the kernel as a freestanding image without libc.

---

# Build the Kernel

```bash
make
```

Main output:

```text
firstos.bin
```

---

# Run with QEMU

```bash
make run
```

The current `run` target builds the required initrd image and launches:

```bash
qemu-system-i386 -kernel firstos.bin -initrd initrd.img -serial stdio
```

as defined by the repository Makefile.

---

# Build an ISO

```bash
make iso
```

This target:

1. builds the kernel
2. builds the user program
3. creates `initrd.img`
4. creates the GRUB configuration
5. generates a bootable ISO with `grub-mkrescue`

Output:

```text
firstos.iso
```

---

# Clean

```bash
make clean
```

This removes generated build artifacts including:

```text
*.o
firstos.bin
firstos.iso
user_prog.elf
initrd.img
iso/
```

---

# Debugging

The kernel provides serial diagnostics alongside VGA console output.

Typical boot messages include:

```text
[ ok ] multiboot verified
[ ok ] kernel console ready
[ ok ] initrd module loaded
[info] initrd start: ...
[info] initrd size: ...
```

The dual-console approach provides a practical debugging path during early boot and user-space initialization.

---

# Current Development Status

firstOS is an experimental operating system kernel under active development.

The repository currently covers a substantial portion of the early operating-system stack, from bootstrapping and hardware initialization through memory management, scheduling, privilege separation, system calls, IPC, and user-space execution.

However, the implementation should still be considered experimental. Correctness, isolation, cleanup, hardware compatibility, and long-term API stability require additional work.

---

# Known Limitations

The project currently has significant limitations, including:

* 32-bit x86 only
* not intended for production use
* limited hardware driver support
* no complete filesystem subsystem
* early-stage user-space API
* early-stage IPC implementation
* ELF execution support is still under development
* memory management is not production-grade
* limited hardware compatibility
* security guarantees are still under development

Running firstOS on physical hardware should therefore be considered experimental.

---

# Roadmap

Potential development directions include:

## Kernel

* improve memory management
* improve page fault handling
* improve process and task lifecycle management
* improve resource cleanup
* add synchronization primitives
* improve timer abstraction
* introduce a more structured driver architecture

## User Space

* improve ELF loading
* improve process lifecycle
* strengthen process isolation
* introduce a user-space runtime
* expand shell functionality
* move test programs out of the kernel test layer

## System Calls

* stabilize the syscall ABI
* add process management
* add file abstractions
* add memory mapping facilities
* introduce process identifiers
* improve error handling

## IPC

* improve message queues
* add synchronization
* formalize process-to-process communication
* define stable IPC semantics

## Storage

* add block-device abstraction
* add a filesystem
* load executables from persistent storage
* introduce a virtual filesystem layer

---

# Design Principles

firstOS follows several design principles:

### 1. Small and Understandable

The system should remain small enough that the relationship between major components can be studied as a whole.

### 2. Explicit Systems Programming

The implementation should make CPU, memory, interrupt, and privilege mechanisms visible rather than hiding them behind unnecessary abstraction.

### 3. Incremental Development

Subsystems are implemented incrementally and validated before the system is expanded further.

### 4. Kernel/User Separation

Kernel and user-space execution should be separated through CPU privilege levels and address-space isolation.

### 5. Testability

Subsystems should provide diagnostics or dedicated test paths that make failures observable.

---

# License

This project is licensed under the MIT License.

See [LICENSE](LICENSE) for the full license text.

---

# Security

Security-related information and the project's security policy are documented separately in [SECURITY.md](SECURITY.md).

---

# Repository

Source code:

https://github.com/chakronwork/Firstkernel
