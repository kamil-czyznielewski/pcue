/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     buzzer_spe.c
* @brief    Buzzer dedicated functions with RTTTL format support.
* @author   IB
* @date     07/2007
*
* @date     09/2008
* @version  3.0 Add audio version for Primer 2
* @note     Split from buzzer.c
*
* @note     Platform = STM32 except STM32L & STM3220G
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static TIM_TimeBaseInitTypeDef   TIM_TimeBaseStructure;
static TIM_OCInitTypeDef         TIM_OCInitStructure;
u16                              CCR_Val              = 0x2EE0;

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/* External variables --------------------------------------------------------*/
extern enum BUZZER_mode Buzzer_Mode;
extern RCC_ClocksTypeDef RCC_ClockFreq;

/***********************************************************************************
*
*                                BUZZER_SetFrequency
*
************************************************************************************/
/**
*
*  Set the buzzer frequency
*
*  @param[in]  freq New frequency.
*
**/
/********************************************************************************/
void BUZZER_SetFrequency( u16 freq )
{

#if !AUDIO_AVAIL
    /* Calculate the frequency (depend on the PCLK1 clock value) */
    CCR_Val = ( RCC_ClockFreq.PCLK1_Frequency / freq );

    TIM_TimeBaseStructure.TIM_Period          = CCR_Val * 2;
    TIM_TimeBaseStructure.TIM_Prescaler       = 0x0;
    TIM_TimeBaseStructure.TIM_ClockDivision   = 0x0;
    TIM_TimeBaseStructure.TIM_CounterMode     = TIM_CounterMode_Up;

    TIM_TimeBaseInit( TIM3, &TIM_TimeBaseStructure );

    /* Output Compare Toggle Mode configuration: Channel3 */
    TIM_OCInitStructure.TIM_OCMode   = TIM_OCMode_PWM1;
    /* in FWLib v1.0 : TIM_OCInitStructure.TIM_Channel  = TIM_Channel_3;*/
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; /* FWLib v2.0*/
    TIM_OCInitStructure.TIM_Pulse    = CCR_Val;

    TIM_OC3Init( TIM3, &TIM_OCInitStructure );  /* FWLib v2.0*/
    TIM_OC3PreloadConfig( TIM3, TIM_OCPreload_Enable );

#else
    AUDIO_BUZZER_SetToneFrequency( freq );
#endif
}

/* Public functions for CircleOS ---------------------------------------------*/

/*******************************************************************************
*
*                                BUZZER_Init
*
*******************************************************************************/
/**
*
*  Buzzer Initialization
*
*  @attention  This function must <b>NOT</b> be called by the user.
*
**/
/******************************************************************************/
NODEBUG2 void BUZZER_Init( void )
{
#if !AUDIO_AVAIL
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable GPIOB clock  */
    RCC_PERIPH_GPIO_CLOCK_CMD( RCC_APB2Periph_GPIOB, ENABLE );

    /* GPIOB Configuration: TIM3 3in Output */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init( GPIOB, &GPIO_InitStructure );

    /* TIM3 Configuration ------------------------------------------------------*/
    /* TIM3CLK = 18 MHz, Prescaler = 0x0, TIM3 counter clock = 18  MHz */
    /* CC update rate = TIM3 counter clock / (2* CCR_Val) ~= 750 Hz */

    /* Enable TIM3 clock */
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM3, ENABLE );
    TIM_DeInit( TIM3 );
    TIM_TimeBaseStructInit( &TIM_TimeBaseStructure );
    TIM_OCStructInit( &TIM_OCInitStructure );

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period          = 0xFFFF;
    TIM_TimeBaseStructure.TIM_Prescaler       = 0x0;
    TIM_TimeBaseStructure.TIM_ClockDivision   = 0x0;
    TIM_TimeBaseStructure.TIM_CounterMode     = TIM_CounterMode_Up;

    TIM_TimeBaseInit( TIM3, &TIM_TimeBaseStructure );

    /* Output Compare Toggle Mode configuration: Channel3 */
    TIM_OCInitStructure.TIM_OCMode   = TIM_OCMode_Toggle;
    /* in FWLib v1.0 : TIM_OCInitStructure.TIM_Channel  = TIM_Channel_3;*/
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; /* now in in FWLib v2.0*/
    TIM_OCInitStructure.TIM_Pulse    = CCR_Val;

    TIM_OC3Init( TIM3, &TIM_OCInitStructure );  /* changed against FWLib v2.0*/
    TIM_OC3PreloadConfig( TIM3, TIM_OCPreload_Disable );
    
    BUZZER_SetFrequency( 440 );
#endif

#if AUDIO_AVAIL
    /* RQ : init done by Audio init*/
#endif

    BUZZER_SetMode( BUZZER_OFF );
}


/// @endcond

/* Public functions ----------------------------------------------------------*/


/*******************************************************************************
*
*                                BUZZER_SetMode
*
*******************************************************************************/
/**
*
*  Set new buzzer mode
*
*  @param[in]  mode  New buzzer mode.
*
**/
/******************************************************************************/
void BUZZER_SetMode( enum BUZZER_mode mode )
{

#if AUDIO_AVAIL
    /* Don't use the buzzer if audio playing */
    if (( AUDIO_Playback_status != NO_SOUND )
            || ( AUDIO_Recording_status != NO_RECORD )
            || ( AUDIO_BuzzerOn == 0 ) )
    {
        mode = BUZZER_OFF;
    }
#endif

    Buzzer_Mode    = mode;
    Buzzer_Counter = 0;

    switch ( mode )
    {
    case BUZZER_PLAYMUSIC   :
#if AUDIO_AVAIL
        AUDIO_BUZZER_OnOff( ON );
#endif
        BUZZER_PlayNote();  /* play melody*/
        /* no break */

    case BUZZER_LONGBEEP    :
    case BUZZER_SHORTBEEP   :
    case BUZZER_ON          :
#if AUDIO_AVAIL
        AUDIO_BUZZER_OnOff( ON );
#else
        TIM_Cmd( TIM3, ENABLE );
#endif
        break;

    case BUZZER_OFF         :
#if AUDIO_AVAIL
        AUDIO_BUZZER_OnOff( OFF );
#else
        TIM_Cmd( TIM3, DISABLE );
#endif
        break;
    }
}

