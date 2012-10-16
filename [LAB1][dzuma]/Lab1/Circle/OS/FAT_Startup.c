/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     FAT_Startup.c
* @brief
* @author   FL
* @date     07/2007
*
**/
/******************************************************************************/

/* Exported constants --------------------------------------------------------*/
extern unsigned long _etext;
extern unsigned long _sidata;    /* start address for the initialization values of the .data section. defined in linker script */
extern unsigned long _sdata;     /* start address for the .data section. defined in linker script */
extern unsigned long _edata;     /* end address for the .data section. defined in linker script */

extern unsigned long _sbss;      /* start address for the .bss section. defined in linker script */
extern unsigned long _ebss;      /* end address for the .bss section. defined in linker script */

extern void _estack;             /* init value for the stack pointer. defined in linker script */

__attribute__(( section( ".isr_vector" ) ) )

long unsigned table [ 1 ] = {0x08006000};

__attribute__(( section( ".b1text" ) ) )

const int ___FAT_Startup_dummy_int = 0xFFFFFFFF;
