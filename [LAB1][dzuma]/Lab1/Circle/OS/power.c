/****************** COPYRIGHT (C) 2007-2011 RAISONANCE S.A.S. *****************/
/**
*
* @file     power.c
* @brief    CircleOS shutdown driver.
* @author   YRT
* @date     09/2009
*
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

#define MAX_TIME_ON (60*5) //shutdown after 5mn when no activity
u8 PWR_BatteryLevel = 100;

u16 PWR_CurrentVBat = 0;
counter_t PWR_ReferenceTime = -1;
enum ePowerState PWR_CurrentState = PWR_STATE_UNDEF;


/*******************************************************************************
*
*                                POWER_Handler
*
*******************************************************************************/
/**
*
*  Called by the CircleOS scheduler to manage the states of the power.
*
*
**/
/******************************************************************************/
void POWER_Handler( void )
{
    static u32 last_time = 0;
    static enum ePowerState last_state = PWR_STATE_UNDEF;
    static u16 MinBat = 0;
    static s32 stacounter = 0;
    static s32 stacounter2 = 0;

    // iToShutDown is a global flag to indicate that a shutdown has been requested
    if ( iToShutDown )
    {
        while ( 1 )
        {
            SHUTDOWN_Action();
        }
    }

    u32 time = RTC_GetCounter();

    if ( last_time == 0 )    // initialization
    {
        PWR_CurrentVBat = UTIL_GetBat();
    }

    //
    if ( time != last_time ) // once per second is enough for measurement
    {
        u16 vbat = UTIL_GetBat();

        // Filter on 8 values
        PWR_CurrentVBat = (( PWR_CurrentVBat * 7 ) + vbat ) / 8;
    }
    else return;

    // Power management
    if (( time / 3 ) != ( last_time / 3 ) ) // once per 3 seconds is enough for changing the display/state
    {
        // First read the charger status
        s32 status1 = ( GPIO_ReadInputDataBit( GPIOx_PWR_STAT, GPIO_PWR_LOADING_PIN ) == Bit_SET );
        s32 status2 = ( GPIO_ReadInputDataBit( GPIOx_PWR_STAT, GPIO_PWR_DONE_PIN ) == Bit_SET );
        enum ePowerState new_state = PWR_STATE_UNDEF;
        s32 flagsta = status1 + 2 * status2;        // Rq: status = 1 when led off

        switch ( flagsta )
        {
        case 1 : // Done : (green on and red off)
            new_state = PWR_STATE_FULL;
            PWR_BatteryLevel   = 100;
            break;
        case 2 : // Battery is being charged : (green off and red on)
            new_state = PWR_STATE_CHARGING;
            PWR_BatteryLevel   = 50;
            break;
        case 3 : // Power supplied by battery : (green off and red off) = normal mode
            new_state = PWR_STATE_NORMAL;
            PWR_BatteryLevel   = 50;
            break;
        case 0 : // Battery is absent or dead : (green on and red on)
            if ( PWR_CurrentVBat < NO_BAT_VOLTAGE )
            {
                // USB connected
                new_state = PWR_STATE_NOBAT;
                PWR_BatteryLevel   = 100;
            }
            else
                // Battery is dead
            {
                new_state = PWR_STATE_NORMAL;
                PWR_BatteryLevel   = 0;
            }
            break;
        }

        // We change only when we have twice the same state
        if ( new_state == last_state )
        {
            if ( new_state != PWR_CurrentState )
            {
                switch ( PWR_CurrentState )
                {
                case PWR_STATE_NORMAL:
                case PWR_STATE_LOW:
                case PWR_STATE_EMPTY:
                    if ( new_state == PWR_STATE_NORMAL )
                        break;
                default:
                    PWR_CurrentState = new_state;
                    MinBat = PWR_CurrentVBat;
                    break;
                }
            }
        }

        // Now manage the power level.
        switch ( PWR_CurrentState )
        {
            case PWR_STATE_NORMAL:
                if ( MinBat < BAT_LOW_VOLTAGE )
                {
                    PWR_CurrentState = PWR_STATE_LOW;
                }
            case PWR_STATE_LOW:
                if ( PWR_CurrentVBat < MinBat )
                {
                    MinBat = ( PWR_CurrentVBat + MinBat ) / 2;
                }
                if ( MinBat < BAT_EMPTY_VOLTAGE )
                {
                    PWR_CurrentState = PWR_STATE_EMPTY;
                }
                PWR_BatteryLevel = ( 100 * ( MinBat - BAT_EMPTY_VOLTAGE ) )  / ( BAT_FULL_VOLTAGE - BAT_EMPTY_VOLTAGE ) ;
                if ( PWR_BatteryLevel  > 100 )
                {
                    PWR_BatteryLevel  = 100 ;
                }
                else  if ( PWR_BatteryLevel  < 0 )
                {
                    PWR_BatteryLevel  = 0 ;
                }
                stacounter = 0;
                break;

            case PWR_STATE_EMPTY:
                // 15" filter before shutdown message
                if ( stacounter2++ > 45 )
                {
                    CurrentMenu = 0;
                    LCD_SetOffset( OFFSET_OFF );
                    LCD_ClearAllScreen( RGB_BLACK );
                    DRAW_SetDefaultColor();
                    DRAW_RestoreCharMagniCoeff();
                    DRAW_DisplayStringWithMode( 0, 90, " BATTERY LOW!!!  ", ALL_SCREEN, INVERTED_TEXT, CENTER );
                    DRAW_DisplayStringWithMode( 0, 60, " Shutting down!  ", ALL_SCREEN, INVERTED_TEXT, CENTER );
                    DRAW_SetCharMagniCoeff( 1 );
                    if ( stacounter++ > 4 )
                    {
                        SHUTDOWN_Action();
                    }
                }
                break;
            
            default:
                break;
            
        } // switch ( PWR_CurrentState )
        
        last_state = new_state;
    
    }   // if (( time / 3 ) != ( last_time / 3 ) )

    last_time = time;


    if ( PWR_ReferenceTime != -1 )
    {
        counter_t On_Time = time - PWR_ReferenceTime;
        if ( On_Time > MAX_TIME_ON )
        {
            if (( PWR_CurrentState != PWR_STATE_CHARGING ) && ( PWR_CurrentState != PWR_STATE_NOBAT ) )
            {
                SHUTDOWN_Action();
            }
            else
            {
                POWER_Set_Time();
            }
        }
    }

}

/*******************************************************************************
*
*                                POWER_Reset_Time
*
*******************************************************************************/
/**
*
*  Resets the time counter
*
*  @note
*
**/
/******************************************************************************/
void POWER_Reset_Time( void )
{
    PWR_ReferenceTime = -1;
}

/*******************************************************************************
*
*                                POWER_Set_Time
*
*******************************************************************************/
/**
*
*  Sets the time counter to the current time
*
*  @note
*
**/
/******************************************************************************/
void POWER_Set_Time( void )
{
    if ( PWR_ReferenceTime == -1 )
    {
        if ( BUTTON_GetMode() == BUTTON_ONOFF_FORMAIN )
            PWR_ReferenceTime = RTC_GetCounter();
    }
}

/// @endcond


