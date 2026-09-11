ENTRY(_start)

SECTIONS
{
    . = 0x40000000;

    .text : ALIGN(0x1000)
    {
        *(.text*)
        *(.rodata*)
        *(.data*)
        *(.bss*)
        *(COMMON)
    }

    /DISCARD/ :
    {
        *(.comment)
        *(.note*)
        *(.eh_frame)
    }
}
