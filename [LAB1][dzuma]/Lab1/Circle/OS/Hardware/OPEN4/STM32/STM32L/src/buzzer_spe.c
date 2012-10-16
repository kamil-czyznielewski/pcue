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
* @date     09/2010
* @note     Split from buzzer.c
* @note     Platform = Open4 STM32L
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
u16                        CCR_Val = 0x2EE0;
// This array will be used to generate a square signal on Audio signal
// Note : the first value determines the audio volume
u16 waveForm[] = {0x0FFF, 0};


/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/* External variables --------------------------------------------------------*/
extern enum BUZZER_mode Buzzer_Mode;

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
    /* Calculate the frequency (depend on the PCLK1 clock value) */
    CCR_Val = ( RCC_ClockFreq.PCLK1_Frequency / freq );

    TIM_TimeBaseStructure.TIM_Period          = CCR_Val / 2;
    TIM_TimeBaseStructure.TIM_Prescaler       = 0x0;
    TIM_TimeBaseStructure.TIM_ClockDivision   = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode     = TIM_CounterMode_Up;

    TIM_TimeBaseInit( TIM_BUZZER, &TIM_TimeBaseStructure );
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
void BUZZER_Init( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;
    DAC_InitTypeDef            DAC_InitStructure;
    DMA_InitTypeDef            DMA_InitStructure;

    /* Enable clocks  */
    RCC_PERIPH_GPIO_CLOCK_CMD( GPIO_BUZZER_PERIPH, ENABLE );
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_DAC, ENABLE );
    RCC_AHBPeriphClockCmd( RCC_AHBPeriph_DMA1, ENABLE );
    RCC_APB1PeriphClockCmd( TIM_BUZZER_PERIPH, ENABLE );

    /* GPIO Configuration for DAC */
    GPIO_InitStructure.GPIO_Pin   = GPIO_BUZZER_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_Init( GPIOx_BUZZER_PORT, &GPIO_InitStructure );

    /* TIM7 Configuration ------------------------------------------------------*/
    /* Time base configuration */
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
    TIM_TimeBaseStructure.TIM_Period = 0xFF;          
    TIM_TimeBaseStructure.TIM_Prescaler = 0x0;       
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;    
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(TIM_BUZZER, &TIM_TimeBaseStructure);

    /* TIM7 TRGO selection */
    TIM_SelectOutputTrigger(TIM_BUZZER, TIM_TRGOSource_Update);

    /* TIM7 enable counter */
    TIM_Cmd(TIM_BUZZER, ENABLE);
  
    // Configure DAC 2 with DMA and TIM7 trigger
    DMA_DeInit(DMA1_Channel3); 
    DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR12R2_Address;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&waveForm;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = COUNTOF( waveForm );
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);

    /* DAC channel2 Configuration */
    DAC_InitStructure.DAC_Trigger = DAC_Trigger_T7_TRGO;
    DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;

    /* Enable DMA1 Channel3 */
    DMA_Cmd(DMA1_Channel3, ENABLE);

    /* DAC Channel2 Init */
    DAC_Init(DAC_Channel_2, &DAC_InitStructure);

    /* Enable DAC Channel2 */
    DAC_Cmd(DAC_Channel_2, ENABLE);

    /* Enable DMA for DAC Channel2 */
    DAC_DMACmd(DAC_Channel_2, ENABLE);
    
    UTIL_SetPll( UTIL_GetPll() );
    BUZZER_SetFrequency( 440 );

    BUZZER_SetMode( BUZZER_OFF );

    // Initializes sound variables
    AUDIO_Init();
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
    /* Don't use the buzzer if buzzer configured OFF */
    if ( !AUDIO_BuzzerOn )
        {
        mode = BUZZER_OFF;
        }

    Buzzer_Mode    = mode;
    Buzzer_Counter = 0;

    switch ( mode )
        {
        case BUZZER_PLAYMUSIC   :
            BUZZER_PlayNote();  /* play melody*/
            /* no break */

        case BUZZER_LONGBEEP    :
        case BUZZER_SHORTBEEP   :
        case BUZZER_ON          :
            TIM_Cmd( TIM_BUZZER, ENABLE );
            break;

        case BUZZER_OFF         :
            TIM_Cmd( TIM_BUZZER, DISABLE );
            break;
        }
    }

