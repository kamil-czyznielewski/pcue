/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     led_spe.c
* @brief    Hardware specific LED management.
* @author   YRT
* @date     09/2009
* @note     Split from led.c
*
* @note     Platform = STM32 
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private defines -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
s32            GreenLED_Counter  = 0;
s32            RedLED_Counter    = 0;
const s32      HalfPeriod_LF     = 200;
const s32      HalfPeriod_HF     = 50;
const s32      Period_LF         = 200 * 2;
const s32      Period_HF         = 50 * 2;

/* External variables ---------------------------------------------------------*/
extern enum LED_mode  GreenLED_mode;
extern enum LED_mode  RedLED_mode;
extern enum LED_mode  GreenLED_newmode;
extern enum LED_mode  RedLED_newmode;

/* Public functions for CircleOS ---------------------------------------------*/

/*******************************************************************************
*
*                                LED_Init
*
*******************************************************************************/
/**
*
*  Initialization of the GPIOs for the LEDs
*
*  @note    Is called by CircleOS startup.
*
**/
/******************************************************************************/
NODEBUG2 void LED_Init( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable LED GPIO clock */
    RCC_PERIPH_GPIO_CLOCK_CMD( RCC_APBxPeriph_GPIOx_LED, ENABLE );

    /* Configure LED pins as output push-pull */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_LED0 | GPIO_Pin_LED1 ;
#ifdef STM32L1XX_MD
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
#else

#ifdef STM32F2XX
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#else
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#endif // STM32F2XX

#endif  // STM32L1XX_MD
    GPIO_Init( GPIOx_LED, &GPIO_InitStructure );

    // Turn off LED's
    GPIO_WriteBit( GPIOx_LED, GPIO_Pin_LED0, BIT_LED_RESET );
    GPIO_WriteBit( GPIOx_LED, GPIO_Pin_LED1, BIT_LED_RESET );

}


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
*  @param[in]  id       A LED_id indicating the LED to take care of.
*
**/
/******************************************************************************/
void LED_Handler_hw( enum LED_id id )
{
    s32            counter;
    enum LED_mode  mode;

    /*/ Choose the right LED parameters.*/
    if ( id == LED_GREEN )
    {
        counter = GreenLED_Counter;
        mode    = GreenLED_newmode;
    }
    else
    {
        counter = RedLED_Counter;
        mode    = RedLED_newmode;
    }

#if 0   /*/debug bouncing*/
    {
        extern s32 Button_Counter;
        if ( id == LED_GREEN )
        {
#define  ANTI_BOUNCING_COUNT     50

            GPIO_WriteBit( GPIOx_LED, GPIO_Pin_LED0, ( Button_Counter >= ANTI_BOUNCING_COUNT )   ? BIT_LED_SET : BIT_LED_RESET );
            return;
        }
    }
#endif  /*/end debug*/

    switch ( mode )
    {
    case LED_OFF         :
    case LED_ON          :
        if ((( id == LED_GREEN ) && ( GreenLED_mode == mode ) ) ||
                (( id == LED_RED ) && ( RedLED_mode   == mode ) ) )
        {
            return;
        }

        if ( id == LED_GREEN )
        {
            GPIO_WriteBit( GPIOx_LED, GPIO_Pin_LED0, ( mode == LED_OFF ) ? BIT_LED_RESET : BIT_LED_SET );

            GreenLED_mode = mode;
        }
        else if ( id == LED_RED )
        {
            GPIO_WriteBit( GPIOx_LED, GPIO_Pin_LED1, ( mode == LED_OFF ) ? BIT_LED_RESET : BIT_LED_SET );

            RedLED_mode = mode;
        }

        counter = -1;
        break;

    case LED_BLINKING_HF :
        counter++;

        if ( counter == HalfPeriod_HF )
        {
            GPIO_WriteBit( GPIOx_LED, ( id == LED_RED ) ? GPIO_Pin_LED1 : GPIO_Pin_LED0, BIT_LED_SET );
        }
        else if (( counter < 0 ) || ( counter >= Period_HF ) )
        {
            GPIO_WriteBit( GPIOx_LED, ( id == LED_RED ) ? GPIO_Pin_LED1 : GPIO_Pin_LED0, BIT_LED_RESET );

            counter = 0;
        }
        break;

    case LED_BLINKING_LF :
        counter++;

        if ( counter == HalfPeriod_LF )
        {
            GPIO_WriteBit( GPIOx_LED, ( id == LED_RED ) ? GPIO_Pin_LED1 : GPIO_Pin_LED0, BIT_LED_SET );
        }

        else if (( counter < 0 ) || ( counter >= Period_LF ) )
        {
            GPIO_WriteBit( GPIOx_LED, ( id == LED_RED ) ? GPIO_Pin_LED1 : GPIO_Pin_LED0, BIT_LED_RESET );

            counter = 0;
        }
        break;

    default              :
        break;
    }

    if ( id == LED_GREEN )
    {
        GreenLED_Counter = counter;
        GreenLED_mode    = mode;
    }
    else
    {
        RedLED_Counter = counter;
        RedLED_mode    = mode;
    }
}

/// @endcond

/* Public functions ----------------------------------------------------------*/

