/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     power_spe.c
* @brief    CircleOS shutdown driver.
* @author   YRT
* @date     09/2009
* @note     Platform = Open4 STM32E / STM32C
*
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/*******************************************************************************
*
*                                POWER_Init
*
*******************************************************************************/
/**
*
*  Initialization of the GPIOs for the POWER management
*
*  @note
*
**/
/******************************************************************************/
void POWER_Init( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable Status GPIO clock */
    RCC_PERIPH_GPIO_CLOCK_CMD( GPIO_PWR_PERIPH_STAT, ENABLE );

    /* Configure status pins as floating input */
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = GPIO_PWR_LOADING_PIN | GPIO_PWR_DONE_PIN;
    GPIO_Init( GPIOx_PWR_STAT, &GPIO_InitStructure );

    POWER_Set_Time();
}

/// @endcond

/*******************************************************************************
*
*                                SHUTDOWN_Action
*
*******************************************************************************/
/**
*
*  Switch off the power supply.
*  Backup system values (mute, volume, input interface...).
*
**/
/******************************************************************************/
void SHUTDOWN_Action( void )
{
    //-- Backup system values --

    // CPU frequency
    u16 baksys2 = UTIL_GetPll() & BKPMASK_S2_PLL;              // 3 bits
    u16 baksys7 = UTIL_ReadBackupRegister( BKP_SYS7 );

    // Option autorun
    baksys2 |= ( AutorunOn ? ( BKPMASK_S2_AUTORUN ) : 0 );

    // Option loudspeaker
    baksys2 |= ( AUDIO_SpeakerOn ? BKPMASK_S2_SPEAKER : 0 );

    // Is it currently mute?
    baksys2 |= ( AUDIO_Mute ? BKPMASK_S2_MUTE : 0 );

    // Volume
    baksys2 |= ( AUDIO_Volume << 8 ) & BKPMASK_S2_VOLUME;

    // Option buzzer
    baksys2 |= ( AUDIO_BuzzerOn ? ( BKPMASK_S2_BUZZER ) : 0 );

    // Option joystick
    baksys2 |= ( JoystickAsInput ? BKPMASK_S2_JOYSTICK : 0 ); 

    // Option Mems
    baksys2 |= ( MemsAsInput ? BKPMASK_S2_MEMS : 0 ); 

    //Option Touchscreen
    baksys7 &= ~BKPMASK_S7_TCHSCR;
    baksys7 |= ( TchscrAsInput ? ( BKPMASK_S7_TCHSCR ) : 0 );      

    UTIL_WriteBackupRegister( BKP_SYS2, baksys2 );
    UTIL_WriteBackupRegister( BKP_SYS7, baksys7 );

    // Backlight
    UTIL_WriteBackupRegister( BKP_BKLIGHT, LCD_GetBackLight() );

    // Font menus
    UTIL_WriteBackupRegister( BKP_SYS4, Menu_Font );

    //-- Stops peripherals --

    // Disable TIM2 Update interrupt (mems)
    TIM_ITConfig( TIM2, TIM_IT_Update, DISABLE );

    // Stop the audio codec
    AUDIO_Shutdown();

    //-- Power Off --
    
    /* Enable Shutdown GPIO clock */
    RCC_PERIPH_GPIO_CLOCK_CMD( GPIO_PWR_PERIPH, ENABLE );
    
    /* Configure pin shutdown as Push-Pull output  */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   = GPIO_PWR_SHUTDOWN_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init( GPIOx_PWR, &GPIO_InitStructure );

    GPIO_WriteBit( GPIOx_PWR, GPIO_PWR_SHUTDOWN_PIN, SET );

    // Wait for the end
    while ( 1 ) {;}
}
