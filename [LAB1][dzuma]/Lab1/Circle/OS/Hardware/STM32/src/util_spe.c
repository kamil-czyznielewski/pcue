/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     Util_spe.c
* @brief    Various harware specific utilities for CircleOS.
* @author   YRT
* @date     05/2011
* @note     Platform Open4 STM32
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private defines -----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
void fct_dummy() {;}

/* External variables ---------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/

/*******************************************************************************
*
*                    delayms
*
*******************************************************************************/
/**
*
*  Delay of about x ms ?
*
*  @note Do not to call by handler, because decremented by systick.
*        In this case use the second one (delay).
*
**/
/******************************************************************************/
void Delayms( u32 nTime )
{
    TimingDelay = nTime;

    while ( TimingDelay != 0 );
}

void delay( s32 count )
{
    s32 i, j;
    for ( i = 0; i < count; i++ )
        for ( j = 0; j < 1300; j++ )
            fct_dummy();
}

/*******************************************************************************
*
*                                starting_delay
*
*******************************************************************************/
/**
*  Delay at the startup, depending on the platform
*
*
**/
/*******************************************************************************/
void starting_delay( void )
{
    delay( 100 );
}

/******************************************************************************
*
*                                Sleep_Call
*
******************************************************************************/
/* To respect Misra rule 3 :  */
/*"Assembly language functions that are called from C should be written as C functions containing only in-line assembly language.  */
/*In-line assembly language should not be embedded in normal C code."  */

void Sleep_Call( void )
{
    asm( "wfi" );
}

/*******************************************************************************
*
*                                UTIL_LoadApp
*
*******************************************************************************/
/**
*
*  Loads an application in the current memory space. The given application
*  code may be retrieved from uSD card (STM32) or from I2C EEPROM(STM8L).
*  @param[in] app is the index of the application to load.
*
**/
/********************************************************************************/
void UTIL_LoadApp( index_t app )
{
}


/*******************************************************************************
*
*                                my_strlen
*
*******************************************************************************/
/**
*  Behaves as the standard ANSI "strlen", return provided string length.
*
*  @param[in]  p The string to evaluate the length.
*
*  @return  The length of the provided string.
*
**/
/******************************************************************************/
len_t my_strlen( const u8* p )
{
    len_t len = 0;

    for ( ; *p; p++ )
    {
        len++;
    }

    return len;
}


/// @endcond

/*******************************************************************************
*
*                    UTIL_GetUsb
*
*******************************************************************************/
/**
*
*  Return the USB connexion state.
*
*  @return The USB connexion state.
*
**/
/******************************************************************************/
u8 UTIL_GetUsb( void )
{
    return ( GPIO_ReadInputDataBit( USB_DISCONNECT_PORT, USB_DISCONNECT_PIN ) == Bit_SET );
}


/*******************************************************************************
*
*                                UTIL_SetIrqHandler
*
*******************************************************************************/
/**
*
*  Redirect an IRQ handler.
*
*  @param[in]  Offs   Address in the NVIC table
*  @param[in]  pHDL   Pointer to the handler.
*
**/
/********************************************************************************/
void UTIL_SetIrqHandler( s32 Offs, tHandler pHDL )
{
    if (( Offs >= 8 ) && ( Offs < 0x150 ) ) // YRT020100505 - Changed for connectivity line
        *( tHandler* )( CIRCLEOS_RAM_BASE + Offs ) = pHDL;
}

/*******************************************************************************
*
*                                UTIL_GetIrqHandler
*
*******************************************************************************/
/**
*
*  Get the current IRQ handler.
*  Since (V1.6) the vector table is relocated in RAM, the vectors can be easily modified
*  by the applications.
*
*  @param[in]  Offs   Address in the NVIC table
*  @return  A pointer to the current handler or 0 in case of an error.
*
**/
/********************************************************************************/
tHandler UTIL_GetIrqHandler( s32 Offs )
{
    if (( Offs >= 8 ) && ( Offs < 0x150 ) ) // YRT020100505 - Changed for connectivity line
        return *( tHandler* )( CIRCLEOS_RAM_BASE + Offs );
    else
        return 0;
}

/*******************************************************************************
*
*                                UTIL_GetAppAddress
*
*******************************************************************************/
/**
*
*  Get the address of an application in the FAT.
*
*  @param[in]  AppName   pointer on the application name.
*  @return     long      address of the application.
*
**/
/********************************************************************************/
uint_t UTIL_GetAppAddress( const u8* AppName )
{
    s32 i;

    for ( i = 0; i < MAXAPP; i++ )
    {
        tMenuItem*  curapp;
        long unsigned addr;

        if (( *ApplicationTable )[ -i ] == APP_VOID )
        {
            break;
        }
        else
        {
            addr   = ( long unsigned )( *ApplicationTable )[ -i ] ;
            addr &= 0x00FFFFFF;
            addr |= 0x08000000;
            curapp = ( tMenuItem* ) addr;
            if ( !strcmp( curapp->Text, AppName ) )
            {
                return addr;
            }
        }
    }

    return 0;
}

