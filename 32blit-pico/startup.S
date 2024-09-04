#include "engine/api_version.h"

.cpu cortex-m0
.thumb

.section .header, "a"

.word 0x54494C42
.word do_render // render
.word _ZN4blit4tickEm // tick
.word _entry_point // init
.word __flash_binary_size
.word 2 // device_id = 2 + padding
.hword BLIT_API_VERSION_MAJOR
.hword BLIT_API_VERSION_MINOR
.word __flash_binary_start // unused

.section .rodata, "a"
.align 4
.global _ZN4blit3apiE
_ZN4blit3apiE:
    .word 0x10000200

.global _ZN4blit8api_dataE
_ZN4blit8api_dataE:
    .word 0x20000000

// based on pico-sdk crt0.S
.section .reset, "ax"

.type _entry_point,%function
.thumb_func
.global _entry_point
_entry_point:
    push {r4, lr}
    // .data copy
    adr r4, data_cpy_table

    // assume there is at least one entry
1:
    ldmia r4!, {r1-r3}
    cmp r1, #0
    beq 2f
    bl data_cpy
    b 1b
2:

    // Zero out the BSS
    ldr r1, =__bss_start__
    ldr r2, =__bss_end__
    movs r0, #0
    b bss_fill_test
bss_fill_loop:
    stm r1!, {r0}
bss_fill_test:
    cmp r1, r2
    bne bss_fill_loop

platform_entry: // symbol for stack traces
    bl do_init
    pop {r4, pc}

data_cpy_loop:
    ldm r1!, {r0}
    stm r2!, {r0}
data_cpy:
    cmp r2, r3
    blo data_cpy_loop
    bx lr

.align 2
data_cpy_table:
.word __etext
.word __data_start__
.word __data_end__

.word 0 // null terminator
