/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     rtc_spe.c
* @brief    RTC clock management and utilities.
* @author   IB
* @date     07/2007
* @version  4.0 Folder reorganization
* @date     08/2010
*
* @note     Platform = STM32L
*
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
// Default initialization date
static CONST_DATA RTC_DateTypeDef Rtc_DateStr = {RTC_Weekday_Sunday, RTC_Month_May, 9, 10};
static CONST_DATA RTC_InitTypeDef RtcInitStr = {RTC_HourFormat_24, 0x7F, 0x00FF};

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
void RTC_Init( void )
    {
    RTC_InitTypeDef   RTC_InitStructure;
    RTC_TimeTypeDef   RTC_TimeStructure;
    RTC_DateTypeDef   RTC_DateStructure;

    /* CK_RTC clock selection */
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_PWR, ENABLE );
    
    /* Allow access to RTC */
    PWR_RTCAccessCmd(ENABLE);
    
    /* Reset Backup Domain */
    RCC_RTCResetCmd(ENABLE);
    RCC_RTCResetCmd(DISABLE);

    RCC_LSEConfig(RCC_LSE_ON);

    /* Wait till LSE is ready */
    GPIO_WriteBit( GPIOx_LED, GPIO_Pin_LED0, Bit_RESET );    // Green

    while ( RCC_GetFlagStatus( RCC_FLAG_LSERDY ) == RESET )
        {}

    GPIO_WriteBit( GPIOx_LED, GPIO_Pin_LED0, Bit_SET );      // Green

    /* Select the RTC Clock Source */
    RCC_RTCCLKConfig( RCC_RTCCLKSource_LSE );
        
    /* Enable the RTC Clock */
    RCC_RTCCLKCmd( ENABLE );
           
    GPIO_WriteBit( GPIOx_LED, GPIO_Pin_LED1, Bit_RESET );    // Red

    /* Wait for RTC registers synchronization */
    RTC_WaitForSynchro();

    GPIO_WriteBit( GPIOx_LED, GPIO_Pin_LED1, Bit_SET );      // Red
 
    RTC_Init_STLib(( RTC_InitTypeDef* )&RtcInitStr );

    if ( fFirstStartup )
        RTC_SetDate( RTC_Format_BIN, ( RTC_DateTypeDef* )&Rtc_DateStr );

    GPIO_WriteBit( GPIOx_LED, GPIO_Pin_LED1, Bit_SET );      // Red
    }

/*******************************************************************************
*
*                                RTC_GetCounter
*
*******************************************************************************/
/**
*
*  Retrieve a seconds counter.
*
**/
/******************************************************************************/
counter_t RTC_GetCounter( void )
    {
    u8 hour, minute, second;

    RTC_GetTime( &hour, &minute, &second );

    return ( hour * 3600 + minute * 60 + second );
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
    RTC_TimeTypeDef Rtc_TimeStr;

    RTC_GetTime_STLib( RTC_Format_BIN, &Rtc_TimeStr );

    *THH = Rtc_TimeStr.RTC_Hours;
    *TMM = Rtc_TimeStr.RTC_Minutes;
    *TSS = Rtc_TimeStr.RTC_Seconds;
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
    RTC_TimeTypeDef RTC_TimeStr;

    RTC_TimeStr.RTC_H12     = RTC_H12_AM;   // Not used, 24 hour format
    RTC_TimeStr.RTC_Hours   = THH;
    RTC_TimeStr.RTC_Minutes = TMM;
    RTC_TimeStr.RTC_Seconds = TSS;
    RTC_SetTime_STLib( RTC_Format_BIN, &RTC_TimeStr );
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
    
