ENTRY(_start)

SECTIONS
{
    . = 0x40000000;

    .text : ALIGN(0x1000)
    {
        *(.text*)
        *(.rodata*)
    }

    /DISCARD/ :
    {
        *(.comment)
        *(.note*)
        *(.eh_frame)
        *(.data*)
        *(.bss*)
    }
}
