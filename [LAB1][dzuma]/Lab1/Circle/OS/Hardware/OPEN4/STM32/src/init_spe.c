/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     init_spe.c
* @brief    Specific CircleOS initialization.
* @author   YRT
* @date     09/2009
* @note     Platform = Open4 STM32C & STM32E
*
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void NVIC_Configuration( void );
void GPIO_Configuration( void );
void RCC_Configuration( void );
void CheckPrimer();

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
void Specific_Init( void )
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
void NVIC_Configuration( void )
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
void GPIO_Configuration( void )
{
#ifdef TIMING_ANALYSIS  /*only for debugging  */
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE );

    /* Configure pins as push-pull output  */
    GPIO_InitStructure.GPIO_Pin   = /*GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 |*/ GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOA, &GPIO_InitStructure );

    /* Pins of 2x10 to output interrupts lines */
//    GPIO_WriteBit( GPIOA, GPIO_Pin_0, Bit_SET );    /* Mems TIM2 interrupt */
//    GPIO_WriteBit( GPIOA, GPIO_Pin_1, Bit_SET );    /* Hard fault access */
//    GPIO_WriteBit( GPIOA, GPIO_Pin_2, Bit_SET );    /* I2C-AUDIO access */
    GPIO_WriteBit( GPIOA, GPIO_Pin_3, Bit_SET );    /* Systick interruption */

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
void TIM_Configuration( void )
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OCInitStructure;

    /* Enable TIM2 clock */
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2, ENABLE );

    /* TIM2 configuration */
    TIM_TimeBaseStructure.TIM_Period          = 0x200; /*0x4AF; */
    TIM_TimeBaseStructure.TIM_Prescaler       = 0x1;
    TIM_TimeBaseStructure.TIM_ClockDivision   = 0x0;
    TIM_TimeBaseStructure.TIM_CounterMode     = TIM_CounterMode_Up;

    TIM_TimeBaseInit( TIM2, &TIM_TimeBaseStructure );

    /* Output Compare Timing Mode configuration: Channel1 */
    TIM_OCInitStructure.TIM_OCMode   = TIM_OCMode_Timing;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;  // YRT110413
    TIM_OCInitStructure.TIM_Pulse    = 0x0;
    TIM_OC1Init( TIM2, &TIM_OCInitStructure );

    // Configure unsused channels, to avoid conflict with other peripherals (like USART2) YRT110413
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Inactive;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
    TIM_OC2Init( TIM2, &TIM_OCInitStructure );
    TIM_OC3Init( TIM2, &TIM_OCInitStructure );
    TIM_OC4Init( TIM2, &TIM_OCInitStructure );

    /* TIM2 enable counter */
    TIM_Cmd( TIM2, ENABLE );

    /* Immediate load of TIM2 Precaler value */
    TIM_PrescalerConfig( TIM2, 0x100, TIM_PSCReloadMode_Immediate );

    /* Clear TIM2 update pending flag */
    TIM_ClearFlag( TIM2, TIM_FLAG_Update );

    /* Enable TIM2 Update interrupt */
    TIM_ITConfig( TIM2, TIM_IT_Update, ENABLE );
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
void RCC_Configuration( void )
{
    __IO uint32_t StartUpCounter = 0, HSEStatus = 0;

    RCC_DeInit();

    /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration ---------------------------*/
    /* Enable HSE */
    RCC->CR |= (( uint32_t )RCC_CR_HSEON );

    /* Wait till HSE is ready and if Time out is reached exit */
    do
    {
        HSEStatus = RCC->CR & RCC_CR_HSERDY;
        StartUpCounter++;
    }
    while (( HSEStatus == 0 ) && ( StartUpCounter != HSEStartUp_TimeOut ) );

    if (( RCC->CR & RCC_CR_HSERDY ) != RESET )
    {
        HSEStatus = ( uint32_t )0x01;
    }
    else
    {
        HSEStatus = ( uint32_t )0x00;
    }

    if ( HSEStatus == ( uint32_t )0x01 )
    {
        /* Enable Prefetch Buffer */
        FLASH->ACR |= FLASH_ACR_PRFTBE;

        /* Flash 2 wait state */
        FLASH->ACR &= ( uint32_t )(( uint32_t )~FLASH_ACR_LATENCY );
        FLASH->ACR |= ( uint32_t )FLASH_ACR_LATENCY_2;

        /* HCLK = SYSCLK */
        RCC->CFGR |= ( uint32_t )RCC_CFGR_HPRE_DIV1;

        /* PCLK2 = HCLK */
        RCC->CFGR |= ( uint32_t )RCC_CFGR_PPRE2_DIV1;

        /* PCLK1 = HCLK / 2 */
        RCC->CFGR |= ( uint32_t )RCC_CFGR_PPRE1_DIV2;

#ifdef STM32F10X_CL
        /* Configure PLLs ------------------------------------------------------*/
        // HSE = 14.7456 MHz
        /* PLL2 configuration: PLL2CLK = (HSE / 4) * 12 = 44.2368 MHz */
        // Source PLLCLK = PLL2
        RCC->CFGR2 &= ( uint32_t )~( RCC_CFGR2_PREDIV2 | RCC_CFGR2_PLL2MUL  |
                                     RCC_CFGR2_PREDIV1 | RCC_CFGR2_PREDIV1SRC );
        RCC->CFGR2 |= ( uint32_t )( RCC_CFGR2_PREDIV2_DIV4 | RCC_CFGR2_PLL2MUL12 |
                                    RCC_CFGR2_PREDIV1SRC_PLL2 | RCC_CFGR2_PREDIV1_DIV4 | RCC_CFGR2_I2S2SRC );

        /* PLL configuration: PLLCLK = (PLL2CLK / 4) * 6.5 = 71.88 MHz */
        RCC->CFGR &= ( uint32_t )~( RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL );
        RCC->CFGR |= ( uint32_t )( RCC_CFGR_PLLSRC_PREDIV1 | RCC_CFGR_PLLMULL6_5 );

        /* PLL3 configuration: PLLCLK = (HSE / 4) * 20 = 73.728 MHz */
        RCC->CFGR2 &= ( uint32_t )~( RCC_CFGR2_PLL3MUL );
        RCC->CFGR2 |= ( uint32_t )( RCC_CFGR2_PLL3MUL20 );
#else
        /*  PLL configuration: PLLCLK = HSE * 6 = 72 MHz */
        RCC->CFGR &= ( uint32_t )(( uint32_t )~( RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL ) );
        RCC->CFGR |= ( uint32_t )( RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PLLMULL6 );
#endif

        /* Set CPU clock and start PLLs*/
        if ( fFirstStartup )
            UTIL_SetPll( SPEED_MEDIUM );
        else
            UTIL_SetPll( UTIL_ReadBackupRegister( BKP_SYS2 ) & BKPMASK_S2_PLL );

    }
    else
    {
        /* If HSE fails to start-up, the application will have wrong clock
            configuration. User can add here some code to deal with this error */

        /* Go to infinite loop */
        while ( 1 )
        {
        }
    }

    /* ADCCLK = PCLK2/6 */
    RCC_ADCCLKConfig( RCC_CFGR_ADCPRE_DIV6 );

    /* Enable GPIOA & GPIOB clock */
//    RCC_PERIPH_GPIO_CLOCK_CMD( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE );

    /* USB clock */
#ifdef STM32F10X_CL
    RCC_OTGFSCLKConfig( RCC_OTGFSCLKSource_PLLVCO_Div3 );
#else
    RCC_USBCLKConfig( RCC_USBCLKSource_PLLCLK_1Div5 );
#endif

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
void SysTick_Configuration( void )
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
void CheckPrimer( void )
{
    /* Check PRIMER type versus Circle OS and stop if discordance   */
    if ( UTIL_GetPrimerType() != PRIMER_TYPE )
    {
        while ( 1 )
            {};
    }
}

/// @endcond