/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     scheduler.c
* @brief    SysTick interrupt handler for the CircleOS project.
* @author   FL
* @author   IB
* @date     07/2007
*
* @version  3.0 Add Primer 2 and ST library v2.0.3
* @date     11/2008 
* @version  4.0 Add Open4 Primer
* @date     10/2009 
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* External variables --------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
void DummyHandler( void );

/* Private variables ---------------------------------------------------------*/
int SystickCnt = 0;

// The scheduler will call all these handlers one after the other upon each
// systick interrupt. The list is dynamically modifiable through the
// UTIL_SetSchHandler() function.

tHandler SchHandler[16 + 1] =
{
#if (MEMS_POINTER && !MEMS_SPEC_MNGT)     // Not necessary if no MEMS menu handling
    // MEMS management.
    MEMS_Handler,
#else
    DummyHandler,
#endif

    // LEDs management.
    LED_Handler,

    // Button management.
    BUTTON_Handler,

    // Buzzer management.
    BUZZER_Handler,

    // Menu management.
    MENU_Handler,

    // Pointer management.
    POINTER_Handler,

    // LCD management.
    LCD_Handler,

    // Screen orientation management.
    DRAW_Handler,

    // RTC management.
    RTC_DisplayTime,

#if AUDIO_AVAIL
    // Audio management
    AUDIO_Handler,
#else
    DummyHandler,
#endif
#if TOUCHSCREEN_AVAIL
    // Touchscreen management
    TOUCHSCR_Handler,

    // Toolbar management
    TOOLBAR_Handler,
#else
    DummyHandler,
    DummyHandler,
#endif
#if POWER_MNGT
    // Power management
    POWER_Handler,
#else
    DummyHandler,
#endif

    //End of list
    ( tHandler )( void* )( -1 )
};

void DummyHandler( void ){;}

/*******************************************************************************
*
*                                SysTick_Handler
*
*******************************************************************************/
/**
*
*  This function handles SysTick interrupt.
*  According to SysTick_Configuration(), this handler is called every 1ms.
*
**/
/******************************************************************************/
void SysTick_Handler( void )
{
    index_t i;

#ifdef TIMING_ANALYSIS     // to debug with a scope
    SetBitTimingAnalysis( 0 );
#endif
    if ( fInitDone && !fIsStandAlone )
    {
        for ( i = 0 ; i < COUNTOF( SchHandler ); i++ )
        {
            if ( SchHandler[i] == ( tHandler )( -1 ) )
            {
                break; //end of list
            }
            else if ( SchHandler[i] != 0 )
                // (handler == 0) means that it has been disabled
            {
                ( SchHandler[i] )();
            }
        }
    }

    // Delay management
    if ( TimingDelay != 0x00 )
    {
        TimingDelay--;
    }

    SystickCnt++;

#ifdef TIMING_ANALYSIS     // to debug with a scope
    SetBitTimingAnalysis( 1 );
#endif
}


