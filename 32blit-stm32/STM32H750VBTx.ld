/*
*****************************************************************************
**

**  File        : LinkerScript.ld
**
**  Abstract    : Linker script for STM32H750VBTx Device with
**                128KByte FLASH, 1056KByte RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
**
**  Distribution: The file is distributed as is, without any warranty
**                of any kind.
**
**  (c)Copyright Ac6.
**  You may use this file as-is or modify it according to the needs of your
**  project. Distribution of this file (unmodified or modified) is not
**  permitted. Ac6 permit registered System Workbench for MCU users the
**  rights to distribute the assembled, compiled & linked contents of this
**  file as part of an application binary file, provided that it is built
**  using the System Workbench for MCU toolchain.
**
*****************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = 0x20020000;    /* end of RAM */
/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x400;      /* required amount of heap  */
_Min_Stack_Size = 0x400; /* required amount of stack */

/* Specify the memory areas */
MEMORY
{
DTCMRAM (xrw)       : ORIGIN = DEFINED(FLASH_TARGET_INT) ? 0x20000000 : 0x20000400,
                      LENGTH = DEFINED(FLASH_TARGET_INT) ? 1K : 127K
/*RAM_D1 (xrw)        : ORIGIN = 0x24000000, LENGTH = 287K*/
LTDC (rw)           : ORIGIN = 0x24047C00, LENGTH = 225K
/*RAM_D2 (xrw)        : ORIGIN = 0x30000000, LENGTH =  63K*/
FRAMEBUFFER (rw)    : ORIGIN = 0x3000FC00, LENGTH = 225K
RAM_D3 (xrw)        : ORIGIN = 0x38000000, LENGTH =  64K
PERSIST (rw)        : ORIGIN = 0x38800000, LENGTH =   4K
ITCMISR (xrw)       : ORIGIN = 0x00000000, LENGTH =   4K
ITCMRAM (xrw)       : ORIGIN = 0x00001000, LENGTH =  58K

MAIN_RAM (xrw)      : ORIGIN = DEFINED(FLASH_TARGET_INT) ? 0x30000000 : 0x24000000,
                      LENGTH = DEFINED(FLASH_TARGET_INT) ? 63K : 362K

API (rw)            : ORIGIN = 0x0000f800, LENGTH = 2K

/* Flash location depends on whether we are building for internal or external flash. */
/* This defaults to external and can be overridden on the linker command line. */
FLASH (rx)          : ORIGIN = DEFINED(FLASH_TARGET_INT) ? 0x08000000 : 0x90000000,
                      LENGTH = DEFINED(FLASH_TARGET_INT) ? 128K : 32768K
}

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    flash_start = .;
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH

  .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >DTCMRAM AT> FLASH

  .flash_end :
  {
    _flash_end = .;
  } > FLASH

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >MAIN_RAM

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >MAIN_RAM

  itcm_data = LOADADDR(.itcm);
  .itcm :
  {
    . = ALIGN(4);
    itcm_text_start = .;
    *(.itcm)         /* ITCM code section */
    . = ALIGN(4);
    itcm_text_end = .;
  } >ITCMRAM AT> FLASH

  .ltdc (NOLOAD):
  {
    . = ALIGN(4);
    __ltdc_start = .;
    KEEP(*(.ltdc))         /* LTDC buffer section */
    __ltdc_end = .;
  } >LTDC

  .api (NOLOAD):
  {
    . = ALIGN(4);
    __api_start = .;
    KEEP(*(.api))         /* api data section */
    __api_end = .;
  } >API

  .persist (NOLOAD):
  {
    . = ALIGN(4);
    __persist_start = .;
    KEEP(*(.persist))         /* Persistent data section */
    __persist_end = .;
  } >PERSIST

  .fb (NOLOAD):
  {
    . = ALIGN(4);
    __fb_start = .;
    KEEP(*(.fb))           /* framebuffer */
    __fb_end = .;
  } >FRAMEBUFFER

  .dma_data (NOLOAD):
  {
    . = ALIGN(4);
    *(.dma_data)           /* dma_data */
  } >RAM_D3

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}

