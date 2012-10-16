/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     Application_Startup.c
* @brief    Application startup used in LD options when creating an
*           CircleOS application.
* @author   Raisonance S.A.S.
* @date     07/2007
*
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle_api.h"

/// @cond Internal

/* External variables --------------------------------------------------------*/
extern tCircleFunc0 Circle_API [];

extern unsigned long _etext;
extern unsigned long _sidata;    /* start address for the initialization values of the .data section. defined in linker script */
extern unsigned long _sdata;     /* start address for the .data section. defined in linker script */
extern unsigned long _edata;     /* end address for the .data section. defined in linker script */

extern unsigned long _sbss;      /* start address for the .bss section. defined in linker script */
extern unsigned long _ebss;      /* end address for the .bss section. defined in linker script */

extern void _estack;             /* init value for the stack pointer. defined in linker script */

tCircleFunc0( *ptrCircle_API )[];

/* function prototypes -------------------------------------------------------*/
static enum MENU_code Application_Entry( void );

enum MENU_code Application_Handler( void );
enum MENU_code Application_Ini( void );

extern char Application_Name[];

/* Variables -----------------------------------------------------------------*/
__attribute__(( section( ".isr_vector" ) ) )

// Menu item of Application Menu for current application.
tMenuItem ApplicationItem =
{
    Application_Name,          /*<! The application name defined in the user application. */
    Application_Entry,         /*<! The application startup that calls the user application initialization function. */
    Application_Handler,       /*<! The application manager. */
    APP_MENU | REMOVE_MENU     /*<! Menu is for an application and must be removed. */
};

/*******************************************************************************
*
*                                Application_Entry
*
*******************************************************************************/
/**
*  @brief   CircleOS application startup.
*
*  It initializes application memory copying data from flash to RAM and filling
*  with zeros the non initialized memeory (NSS) and calls the user defined
*  application initialization function.
*
*  @return  Value returned by user application initialization function.
*
**/
/******************************************************************************/
static enum MENU_code Application_Entry( void )
{
    unsigned long* pulSrc;
    unsigned long* pulDest;

    //
    // Copy the data segment initializers from flash to SRAM.
    //
    pulSrc = &_sidata;

    for ( pulDest = &_sdata; pulDest < &_edata; )
    {
        *( pulDest++ ) = *( pulSrc++ );
    }

    //
    // Zero fill the bss segment.
    //
    for ( pulDest = &_sbss; pulDest < &_ebss; )
    {
        *( pulDest++ ) = 0;
    }

    ptrCircle_API =  * (( tCircleFunc0( ** )[] ) 0x0100 );

    //
    // Call the application's entry point.
    //
    return Application_Ini();
}

__attribute__(( section( ".b1text" ) ) )

const unsigned long dummy = 0xFFFFFFFF;


/// @endcond