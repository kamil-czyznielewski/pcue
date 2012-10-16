/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     init_spe.c
* @brief    Specific CircleOS initialization.
* @author   YRT
* @date     09/2009
* @note     Split from main.c
*
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void NVIC_Configuration( void );
void GPIO_Configuration( void );
void CheckPrimer();

/* External functions --------------------------------------------------------*/
extern void RCC_Configuration( void );
extern void SysTick_Configuration( void );

/// @cond Internal

/******************************************************************************
*
*                                Specific_Init
*
*******************************************************************************/
/**
*  Configures specific hardware and check primer type.
*
**/
/******************************************************************************/
NODEBUG void Specific_Init( void )
{
    /* RCC configuration */
    RCC_Configuration();

    /* Check PRIMER versus Circle OS */
    CheckPrimer();

    /* NVIC configuration */
    NVIC_Configuration();

    /* Configure GPIOs */
    GPIO_Configuration();
}

/******************************************************************************
*
*                                NVIC_Configuration
*
*******************************************************************************/
/**
*  Configures Vector Table base location.
*
**/
/******************************************************************************/
NODEBUG void NVIC_Configuration( void )
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Set the Vector Table base location in RAM  */
    NVIC_SetVectorTable( NVIC_VectTab_RAM, CIRCLEOS_RAM_OFS );

    /* Configure two bits for preemption priority */
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_2 );

    /* Enable the TIM2 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                    = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 1; /* Mems priority  */
    NVIC_InitStructure.NVIC_IRQChannelSubPriority         = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                 = ENABLE;

    NVIC_Init( &NVIC_InitStructure );

    /* Systick priority */
    //NVIC_SystemHandlerPriorityConfig( SystemHandler_SysTick, 3, 3 );  /*6,6   */
    NVIC_SetPriority( SysTick_IRQn, 3 );
}

/******************************************************************************
*
*                                GPIO_Configuration
*
******************************************************************************/
/**
*  Configures the used GPIO pins.
*
**/
/******************************************************************************/
NODEBUG void GPIO_Configuration( void )
{

#ifdef TIMING_ANALYSIS  /*only for debugging  */
#ifdef PRIMER1

    /* Configure PC.08 as push-pull output */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7 | GPIO_Pin_6 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOA, &GPIO_InitStructure );

    /* Pins of 2x10 to output interrupts lines */
    GPIO_WriteBit( GPIOA, GPIO_Pin_5, Bit_SET );    /* Systick interrupt */
    GPIO_WriteBit( GPIOA, GPIO_Pin_6, Bit_SET );    /* Systick interrupt */
    GPIO_WriteBit( GPIOA, GPIO_Pin_7, Bit_SET );    /* Mems interrupt */
#endif

#ifdef PRIMER2

    /* Configure PC.08 as push-pull output  */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOA, &GPIO_InitStructure2 );

    /* Pins of 2x10 to output interrupts lines */
    GPIO_WriteBit( GPIOA, GPIO_Pin_0, Bit_SET );    /* Mems interruption */
    GPIO_WriteBit( GPIOA, GPIO_Pin_1, Bit_SET );    /* SPI-MEMS access */
    GPIO_WriteBit( GPIOA, GPIO_Pin_2, Bit_SET );    /* I2C-AUDIO access */
    GPIO_WriteBit( GPIOA, GPIO_Pin_3, Bit_SET );    /* Systick interruption */
#endif
#endif
}

/******************************************************************************
*
                                TIM_Configuration
*
******************************************************************************/
/**
*  Configures the used Timers.
*
**/
/******************************************************************************/
NODEBUG void TIM_Configuration( void )
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OCInitStructure;

    /* TIM2 configuration */
    TIM_TimeBaseStructure.TIM_Period          = 0x200; /*0x4AF; */
    TIM_TimeBaseStructure.TIM_Prescaler       = 0x1;
    TIM_TimeBaseStructure.TIM_ClockDivision   = 0x0;
    TIM_TimeBaseStructure.TIM_CounterMode     = TIM_CounterMode_Up;

    TIM_TimeBaseInit( TIM2, &TIM_TimeBaseStructure );

    /* Output Compare Timing Mode configuration: Channel1 */
    TIM_OCInitStructure.TIM_OCMode   = TIM_OCMode_Timing;
    /* in FWLib v1.0 : TIM_OCInitStructure.TIM_Channel  = TIM_Channel_1; */
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; /* now in FWLib v2.0 */
    TIM_OCInitStructure.TIM_Pulse    = 0x0;
    TIM_OC1Init( TIM2, &TIM_OCInitStructure ); /* changed against FWLib v2.0 */

    /* TIM2 enable counter */
    TIM_Cmd( TIM2, ENABLE );

    /* Immediate load of TIM2 Precaler value */
    TIM_PrescalerConfig( TIM2, 0x100, TIM_PSCReloadMode_Immediate );

    /* Clear TIM2 update pending flag */
    TIM_ClearFlag( TIM2, TIM_FLAG_Update );

    /* Enable TIM2 Update interrupt */
    TIM_ITConfig( TIM2, TIM_IT_Update, ENABLE );

#if PRIMER2
    Check_CPU_Clock();
#endif
}

/*******************************************************************************
*
*                                RCC_Configuration
*
*******************************************************************************/
/**
*
*  clocks settings.
*
**/
/******************************************************************************/
NODEBUG void RCC_Configuration( void )
{

    /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration */

    /* RCC system reset(for debug purpose) */
    RCC_DeInit();

    /* Enable HSE */
    RCC_HSEConfig( RCC_HSE_ON );

    /* Wait till HSE is ready */
    while ( RCC_GetFlagStatus( RCC_FLAG_HSERDY ) == RESET )
        {;}

    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd( FLASH_PrefetchBuffer_Enable );

    /* Flash 2 wait state */
    FLASH_SetLatency( FLASH_Latency_2 );

    /* HCLK = SYSCLK */
    RCC_HCLKConfig( RCC_SYSCLK_Div1 );

    /* PCLK2 = HCLK */
    RCC_PCLK2Config( RCC_HCLK_Div1 );

    /* PCLK1 = HCLK/2 */
    RCC_PCLK1Config( RCC_HCLK_Div2 );

    /* ADCCLK = PCLK2/6 */
    RCC_ADCCLKConfig( RCC_PCLK2_Div6 );

    /* Flash 2 wait state */
    *( vu32* )0x40022000 = 0x02;

    /* Set CPU clock */
    if ( fFirstStartup )
        UTIL_SetPll( SPEED_MEDIUM );
    else
        UTIL_SetPll( UTIL_ReadBackupRegister( BKP_SYS2 ) & BKPMASK_S2_PLL );

    /* Enable GPIOB & GPIOB clock */
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE );

    /* Enable GPIOB clock */
    //RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE );

    /* Enable TIM2 */
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2, ENABLE );
}

/*******************************************************************************
*
*                                SysTick_Configuration
*
*******************************************************************************/
/**
*
*  Configures Systick interrupt.
*
**/
/******************************************************************************/
NODEBUG void SysTick_Configuration( void )
{
    /* Select AHB clock(HCLK) as SysTick clock source */
    SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK_Div8 );

    /* Configure SysTick to generate an interrupt @1.5KHz (with HCLK = 36MHz) */
    if ( SysTick_Config( 36000000 / 1500 ) ) 
    {
        /* Capture error */
        while ( 1 );
    }
}

/******************************************************************************
*
*                                CheckPrimer
*
******************************************************************************/
/**
*  Check the type of PRIMER versus Circle OS version.
*  Stops if it mismatches (if not, CircleOS-P1 shuts down P2...)
*
**/
/******************************************************************************/
NODEBUG void CheckPrimer( void )
{
    int counter = 0;

    /* Check PRIMER type versus Circle OS and stop if discordance   */
#ifdef PRIMER1
    if ( UTIL_GetPrimerType() != PRIMER_TYPE )
    {
        /* This is not a PRIMER 1, stops and alert user by LED's */

        /* Configure LED pins as output push-pull */
        GPIO_InitTypeDef GPIO_InitStructure;
        RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOE, ENABLE );
        GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1 ;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init( GPIOE, &GPIO_InitStructure );

        /* Do flashing the LED's */
        while ( 1 )
        {
            counter++;
            if ( counter == 100000 )
            {
                GPIO_WriteBit( GPIOE, GPIO_Pin_0, Bit_SET );
                GPIO_WriteBit( GPIOE, GPIO_Pin_1, Bit_RESET );
            }
            else if (( counter < 0 ) || ( counter >= 200000 ) )
            {
                GPIO_WriteBit( GPIOE, GPIO_Pin_0, Bit_RESET );
                GPIO_WriteBit( GPIOE, GPIO_Pin_1, Bit_SET );
                counter = 0;
            }
        }
    }
#endif
#ifdef PRIMER2
    if ( UTIL_GetPrimerType() != PRIMER_TYPE )
    {
        while ( 1 )
            {};
    }
#endif
}

/// @endcond