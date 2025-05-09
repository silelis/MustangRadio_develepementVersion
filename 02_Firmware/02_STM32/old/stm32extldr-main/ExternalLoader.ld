/*
*****************************************************************************
**
**  File        : ExternalLoader.ld
**
**  Abstract    : Linker script for STM32H7xx Device with
**                2048KByte FLASH, 128KByte RAM
**
**  Copyright (c) 2020-2021 Clive One / sourcer32@gmail.com
**
*****************************************************************************
*/

/* Entry Point */
ENTRY(Init)

/* Specify the memory areas */
MEMORY
{
  RAM_INFO (r)   : ORIGIN = 0, LENGTH = 1K
  RAM_PROG (xrw) : ORIGIN = 0x24000004, LENGTH = 128K-4 /* 0x20000004 for Non-H7 */
}

/* Define output sections */
SECTIONS
{
  .info :
  {
    KEEP (*(.rodata.StorageInfo))
  } >RAM_INFO

  /* The program code and other data goes into IMAGE */
  .prog :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    /* Ensure entry points are pulled in here */
    KEEP (*(.text.Init))
    KEEP (*(.text.DeInit))
    KEEP (*(.text.Write))
    KEEP (*(.text.Read))
    KEEP (*(.text.Verify))
    KEEP (*(.text.Checksum))
    KEEP (*(.text.SectorErase))
    KEEP (*(.text.MassErase))

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
  } >RAM_PROG

  /* Constant data goes into IMAGE */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >RAM_PROG

  .ARM.extab :
  {
     *(.ARM.extab* .gnu.linkonce.armextab.*)
  } >RAM_PROG

  .ARM :
  {
    *(.ARM.exidx*)
  } >RAM_PROG

  .preinit_array :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >RAM_PROG

  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >RAM_PROG

  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >RAM_PROG

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
    . = ALIGN(4);
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */
    . = ALIGN(4);
  } >RAM_PROG

  /* Uninitialized data section */
  .bss :
  {
    . = ALIGN(4);
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
  } >RAM_PROG

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}

