/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     led.c
* @brief    Common LED management.
* @author   IB
* @date     07/2007
* @version  4.0 Add Open4 Primer
* @date     10/2009
*
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private defines -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
enum LED_mode GreenLED_mode    = LED_UNDEF;
enum LED_mode RedLED_mode      = LED_UNDEF;
enum LED_mode GreenLED_newmode = LED_OFF;
enum LED_mode RedLED_newmode   = LED_OFF;


/* Public functions for CircleOS ---------------------------------------------*/


/*******************************************************************************
*
*                                LED_Handler
*
*******************************************************************************/
/**
*
*  Called by the CircleOS scheduler to manage the states of the LEDs.
*  LEDs may be on, off or blinking according to their state.
*
**/
/******************************************************************************/
void LED_Handler( void )
{
    LED_Handler_hw( LED_GREEN );
    LED_Handler_hw( LED_RED );
}

/// @endcond

/* Public functions ----------------------------------------------------------*/

/*******************************************************************************
*
*                                LED_Set
*
*******************************************************************************/
/**
*
*  Set a specified LED in a specified mode.
*
*  @param[in]  id    A LED_id specifying the LED to change the mode.
*  @param[in]  mode  A LED_mode describing the new LED mode.
*
**/
/******************************************************************************/
void LED_Set( enum LED_id id, enum LED_mode mode )
{
    if ( id == LED_GREEN )
    {
        GreenLED_newmode = mode;
    }
    else if ( id == LED_RED )
    {
        RedLED_newmode = mode;
    }
}
