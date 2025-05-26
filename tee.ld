MEMORY 
{ 
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 16K
    FLASH_APP (rx) : ORIGIN = 0x08004000, LENGTH = 368K
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 96K 
}
_Min_Heap_Size = 0x200;
_Min_Stack_Size = 0x400;

_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS 
{ 
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector)) /* Startup code */
        . = ALIGN(4);
    } >FLASH
    
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

    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)         /* .rodata sections (constants, strings, etc.) */
        *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
        . = ALIGN(4);
    } >FLASH

    _sidata = LOADADDR(.data);
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
    } >RAM

    /* User_heap_stack section, used to check that there is enough RAM left */
    ._user_heap_stack :
    {
        . = ALIGN(8);
        PROVIDE ( end = . );
        PROVIDE ( _end = . );
        . = . + _Min_Heap_Size;
        . = . + _Min_Stack_Size;
        . = ALIGN(8);
    } >RAM

    /* Remove information from the standard libraries */
    /DISCARD/ :
    {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }

    .ARM.attributes 0 : 
    { 
        *(.ARM.attributes) 
    }

    .kms : 
    { 
        . = ALIGN(4);
        KEEP((.kms))
        . = ALIGN(4);
    } > FLASH

    .boot : 
    { 
        . = ALIGN(4);
        KEEP((.boot))
        . = ALIGN(4);
    } > FLASH 
}

/* TEE sections */
/* _stee      = 0x08000000;
_shash     = 0x08000100;
_sauth_tag = 0x08000120;
_sboot     = 0x08000220; */

/* START User App */
/* _app_start = 0x08001000; */

  /* The savety area .tee: */
  /* . = ALIGN(8);             
  _stee = .;                

  .tee :
  {
    KEEP(*(.tee))
    . = ALIGN(8);           
  } > FLASH

  _etee = .;                 */

  /* Size check section .tee */
  /* ASSERT((_etee - _stee) <= 1024, "ERROR: .tee section too big! (max 1024 bytes)")
  ASSERT((_sboot - _stee) >= 0, "TEE layout overlap error!")
  ASSERT((_app_start - _sauth_tag) >= 0x100, "Too small gap after .auth_tag") */

  /* START User App */
  /* . = _app_start; */