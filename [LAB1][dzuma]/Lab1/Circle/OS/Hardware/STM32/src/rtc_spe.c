/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     rtc_spe.c
* @brief    RTC clock management and utilities.
* @author   IB
* @date     07/2007
* @version  4.0 Folder reorganization
* @date     05/2010
*
* @note     Platform = Primer1&2, EvoPrimer STM32E & STM32C
*
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private define ------------------------------------------------------------*/
//#define BUG_RTC_CRYSTAL   // YRT20091112

/* Private variables ---------------------------------------------------------*/

/* Public functions for CircleOS ---------------------------------------------*/

/*******************************************************************************
*
*                                RTC_Init
*
*******************************************************************************/
/**
*
*  Initialize RTC. Called at CircleOS startup.
*
*  @attention  This function must <b>NOT</b> be called by the user.
*
**/
/******************************************************************************/
NODEBUG2 void RTC_Init( void )
{
    /* CK_RTC clock selection */
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE );

    /* Allow access to BKP Domain */
    PWR_BackupAccessCmd( ENABLE );

    /* Enable the LSE OSC */
    RCC_LSEConfig( RCC_LSE_ON );

    /* Disable the LSI OSC */
//    RCC_LSICmd( DISABLE );    //__YRT20091112

    /* Wait till LSE is ready */
#ifndef BUG_RTC_CRYSTAL
#if LED_INV
    GPIO_WriteBit( GPIOx_LED, GPIO_Pin_LED0, Bit_RESET );   // Green
#else
    GPIO_WriteBit( GPIOx_LED, GPIO_Pin_LED0, Bit_SET );     // Green
#endif

    while ( RCC_GetFlagStatus( RCC_FLAG_LSERDY ) == RESET )
        {}

#if LED_INV
    GPIO_WriteBit( GPIOx_LED, GPIO_Pin_LED0, Bit_SET );   // Green
#else
    GPIO_WriteBit( GPIOx_LED, GPIO_Pin_LED0, Bit_RESET );     // Green
#endif
#endif

    /* Select the RTC Clock Source */
    RCC_RTCCLKConfig( RCC_RTCCLKSource_LSE );

    /* Enable the RTC Clock */
    RCC_RTCCLKCmd( ENABLE );

#ifndef BUG_RTC_CRYSTAL
#if LED_INV
    GPIO_WriteBit( GPIOx_LED, GPIO_Pin_LED1, Bit_RESET );   // Red
#else
    GPIO_WriteBit( GPIOx_LED, GPIO_Pin_LED1, Bit_SET );     // Red
#endif

    /* Wait for RTC registers synchronization */
    RTC_WaitForSynchro();

#if LED_INV
    GPIO_WriteBit( GPIOx_LED, GPIO_Pin_LED1, Bit_SET );       // Red
#else
    GPIO_WriteBit( GPIOx_LED, GPIO_Pin_LED1, Bit_RESET );     // Red
#endif
#endif

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

    /* Enable the RTC Second interrupt */
    RTC_ITConfig( RTC_IT_SEC, ENABLE );

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

    /* Set RTC prescaler: set RTC period to 1sec */
    /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
    RTC_SetPrescaler( 32767 );

#ifndef BUG_RTC_CRYSTAL
#if LED_INV
    GPIO_WriteBit( GPIOx_LED, GPIO_Pin_LED1, Bit_RESET );   // Red
#else
    GPIO_WriteBit( GPIOx_LED, GPIO_Pin_LED1, Bit_SET );     // Red
#endif

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

#if LED_INV
    GPIO_WriteBit( GPIOx_LED, GPIO_Pin_LED1, Bit_SET );       // Red
#else
    GPIO_WriteBit( GPIOx_LED, GPIO_Pin_LED1, Bit_RESET );     // Red
#endif
#endif
}

/// @endcond

/* Public functions ----------------------------------------------------------*/

/*******************************************************************************
*
*                                RTC_GetTime
*
*******************************************************************************/
/**
*
*  Return current time.
*
*  @param[out] THH    Current hour.
*  @param[out] TMM    Current minute.
*  @param[out] TSS    Current second.
*
**/
/******************************************************************************/
void RTC_GetTime( u8* THH, u8* TMM, u8* TSS )
{
    counter_t Tmp;

    /* Load the Counter value */
    Tmp = RTC_GetCounter();

    /* Compute  hours */
    *THH = ( Tmp / 3600 ) % 24;

    /* Compute minutes */
    *TMM = ( Tmp / 60 ) % 60;

    /* Compute seconds */
    *TSS = Tmp % 60;
}

/*******************************************************************************
*
*                                RTC_SetTime
*
*******************************************************************************/
/**
*
*  Set current time.
*
*  @param[in] THH    Current hour.
*  @param[in] TMM    Current minute.
*  @param[in] TSS    Current second.
*
**/
/******************************************************************************/
void RTC_SetTime( u8 THH, u8 TMM, u8 TSS )
{
    /* Adjust the counter value */
    RTC_SetCounter( THH * 3600 + TMM * 60 + TSS );
}

/*******************************************************************************
*
*                                RTC_DisplayTime
*
*******************************************************************************/
/**
*
*  Display current time on the 6th line at column 0.
*
*  @see DRAW_DisplayTime
*
**/
/******************************************************************************/
void RTC_DisplayTime( void )
{
    static counter_t last_time = 0;
    counter_t new_time = RTC_GetCounter();

    // Time and/or Vbat

    if ( last_time != new_time )
    {
        last_time = new_time ;

        if ( fDisplayTime == 1 )
        {
            DRAW_DisplayTime( 6, 0 ) ;

            #if DISPLAY_TEMP
            UTIL_SetTempMode(0);
            DRAW_DisplayTemp( 90, 0 );
            #endif
        }
    }
}
