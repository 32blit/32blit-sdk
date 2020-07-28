/*
  ******************************************************************************
  * @file      startup_stm32h750xx.s
  * @author    MCD Application Team
  * @brief     STM32H750xx Devices vector table for GCC based toolchain.
  *            This module performs:
  *                - Set the initial SP
  *                - Set the initial PC == do_init,
  *                - Set the vector table entries with the exceptions ISR address
  *                - Branches to main in the C library (which eventually
  *                  calls main()).
  *            After Reset the Cortex-M processor is in Thread mode,
  *            priority is Privileged, and the Stack is set to Main.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
*/

  .syntax unified
  .cpu cortex-m7
  .fpu softvfp
  .thumb

.extern init
.extern update
.extern render

.global  g_pfnVectors


// start address for the initialization values of the .data section.
// defined in linker script
.word  _sidata
// start address for the .data section. defined in linker script
.word  _sdata
// end address for the .data section. defined in linker script
.word  _edata
// start address for the .bss section. defined in linker script
.word  _sbss
// end address for the .bss section. defined in linker script
.word  _ebss
// stack used for SystemInit_ExtMemCtl; always internal RAM used


  .section  .text.do_init
  .weak  do_init
  .type  do_init, %function
do_init:


// Copy the data segment initializers from flash to SRAM
  movs  r1, #0
  b  LoopCopyDataInit

CopyDataInit:
  ldr  r3, =_sidata
  ldr  r3, [r3, r1]
  str  r3, [r0, r1]
  adds  r1, r1, #4

LoopCopyDataInit:
  ldr  r0, =_sdata
  ldr  r3, =_edata
  adds  r2, r0, r1
  cmp  r2, r3
  bcc  CopyDataInit

  ldr  r2, =_sbss
  b  LoopFillZerobss

// Zero fill the bss segment.
FillZerobss:
  movs  r3, #0
  str  r3, [r2], #4

LoopFillZerobss:
  ldr  r3, = _ebss
  cmp  r2, r3
  bcc  FillZerobss

  push {lr}
// Call static constructors
  bl __libc_init_array
// Call the application's entry point.
  bl cpp_do_init
  pop {pc}

/*****************************************************************************
*
* The minimal vector table for a Cortex M. Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
*
*******************************************************************************/

 .section  .isr_vector,"a",%progbits
  #.type  g_pfnVectors, %object
  #.size  g_pfnVectors, .-g_pfnVectors

g_pfnVectors:
  .word  0x54494C42
  .word  _Z6renderm
  .word  _ZN4blit4tickEm
  .word  do_init
  .word flash_start

/*
.weak      render
.thumb_set render,main

.weak      update
.thumb_set update,main

.weak      init
.thumb_set init,main
*/