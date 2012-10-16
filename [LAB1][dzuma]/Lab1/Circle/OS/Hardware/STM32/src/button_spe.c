/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     button_spe.c
* @brief    Button initialization and management. Hardware level
* @author   YRT
* @date     09/2009
*
* @note     Split from button.c
*
* @note   Platform : STM32
*
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
enum JOYSTICK_state JOYSTICK_GetNewState( void );

/* Extern variables ---------------------------------------------------------*/
extern enum BUTTON_mode    Button_Mode;

/* Public functions for CircleOS ---------------------------------------------*/

/*******************************************************************************
*
*                                BUTTON_Init
*
*******************************************************************************/
/**
*
*  General initialization of the GPIO for the button.
*
*  @attention  This function must <b>NOT</b> be called by the user.
*
**/
/******************************************************************************/
NODEBUG2 void BUTTON_Init( void )
{
    /* Enable BUTTON GPIO clock */
    RCC_PERIPH_GPIO_CLOCK_CMD( GPIO_BUTTON_PERIPH, ENABLE );

    /* Configure pin as floating input */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   = GPIO_BUTTON_PIN;

#ifdef STM32L1XX_MD
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
#else

#ifdef STM32F2XX
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#else
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#endif // STM32F2XX

#endif // STM32L1XX_MD
    GPIO_Init( GPIOx_BUTTON, &GPIO_InitStructure );

#if JOYSTICK_AVAIL
    RCC_PERIPH_GPIO_CLOCK_CMD( GPIO_JOYSTICK_PERIPH, ENABLE );

    GPIO_InitStructure.GPIO_Pin   = GPIO_JOYSTICK_L_PIN | GPIO_JOYSTICK_R_PIN | GPIO_JOYSTICK_U_PIN | GPIO_JOYSTICK_D_PIN;
#ifdef STM32L1XX_MD
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
#else

#ifdef STM32F2XX
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#else
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#endif // STM32F2XX

#endif // STM32L1XX_MD
    GPIO_Init( GPIOx_JOYSTICK, &GPIO_InitStructure );
#endif // JOYSTICK_AVAIL

    Button_Mode = BUTTON_ONOFF_FORMAIN;

    /* Restore the interface configuration*/
    if ( fFirstStartup == FALSE )
    {
        TchscrAsInput = ( UTIL_ReadBackupRegister( BKP_SYS7 ) ) & BKPMASK_S7_TCHSCR;
        if ( TchscrAsInput )
        {
            JoystickAsInput = 0;
            MemsAsInput = 0;
        }
        else
        {

            JoystickAsInput = ( UTIL_ReadBackupRegister( BKP_SYS2 ) ) & BKPMASK_S2_JOYSTICK;
            if ( JoystickAsInput == 0 )
            {
                MemsAsInput = 1;
            }
            else
            {
                MemsAsInput = ( UTIL_ReadBackupRegister( BKP_SYS2 ) ) & BKPMASK_S2_MEMS;
            }
        }
    }
    /* Or put the default configuration (mems + joystick)*/
    else
    {
        TchscrAsInput = 0;
        MemsAsInput = JoystickAsInput = 1;
    }
}


/*******************************************************************************
*
*                                 BUTTON_GetNewState
*
*******************************************************************************/
/**
* Decodes the Button state.
*
* @param[in]    : None.
* @param[out]   : None.
* @return       : True if button pushed.
*
*******************************************************************************/
u8 BUTTON_GetNewState( void )
{
    return ( GPIO_ReadInputDataBit( GPIOx_BUTTON, GPIO_BUTTON_PIN ) == Bit_SET );
}

/*******************************************************************************
*
*                                 JOYSTICK_GetNewState
*
*******************************************************************************/
/**
* Decodes the Joystick direction.
*
* @param[in]    : None.
* @param[out]   : None.
* @return       : The direction value.
*
*******************************************************************************/
enum JOYSTICK_state JOYSTICK_GetNewState( void )
{
    enum JOYSTICK_state abs_direction = JOYSTICK_RELEASED;
    enum JOYSTICK_state direction = JOYSTICK_RELEASED;

#if JOYSTICK_AVAIL
    /* "Right" key is pressed */
    if ( GPIO_ReadInputDataBit( GPIOx_JOYSTICK, GPIO_JOYSTICK_R_PIN ) )
    {
        abs_direction = JOYSTICK_RIGHT;
    }

    /* "Left" key is pressed */
    if ( GPIO_ReadInputDataBit( GPIOx_JOYSTICK, GPIO_JOYSTICK_L_PIN ) )
    {
        abs_direction = JOYSTICK_LEFT;
    }

    /* "Up" key is pressed */
    if ( GPIO_ReadInputDataBit( GPIOx_JOYSTICK, GPIO_JOYSTICK_U_PIN ) )
    {
        abs_direction = 3 * abs_direction + JOYSTICK_UP;
    }

    /* "Down" key is pressed */
    if ( GPIO_ReadInputDataBit( GPIOx_JOYSTICK, GPIO_JOYSTICK_D_PIN ) )
    {
        abs_direction = 3 * abs_direction + JOYSTICK_DOWN;
    }
#endif /*JOYSTICK_AVAIL*/

    if ( abs_direction != JOYSTICK_RELEASED )
    {

        /* Adapt value to screen position*/
        /* Note : the joystick is rotated 90° to the right versus LCD orientation */
        switch ( LCD_GetScreenOrientation() )
        {
        case V12:
            direction = JOYSTICK_CircularPermutation( abs_direction , 1 );
            break;
        case V9:
            direction = abs_direction;
            break;
        case V3:
            direction = JOYSTICK_CircularPermutation( abs_direction , 2 );
            break;
        case V6:
            direction = JOYSTICK_CircularPermutation( abs_direction , 3 );
            break;
        }
    }
    else
        direction = JOYSTICK_RELEASED;


    return direction;

}

/// @endcond



