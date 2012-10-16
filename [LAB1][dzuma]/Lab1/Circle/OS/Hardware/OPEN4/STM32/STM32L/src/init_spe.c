/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     init_spe.c
* @brief    Specific CircleOS initialization.
* @author   YRT
* @date     08/2010
* @note     Platform = Open4 STM32L
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

    /* Configure pins as push-pull output  */
    GPIO_InitStructure.GPIO_Pin   = /*GPIO_Pin_4 | GPIO_Pin_1 */ GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_Init( GPIOA, &GPIO_InitStructure );

    /* Pins of 2x10 to output interrupts lines */
//    GPIO_WriteBit( GPIOA, GPIO_Pin_4, Bit_SET );    /* Mems TIM2 interrupt */
//    GPIO_WriteBit( GPIOA, GPIO_Pin_1, Bit_SET );    /* Hard fault */
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
    RCC->CR |= ((uint32_t)RCC_CR_HSEON);

    /* Wait till HSE is ready and if Time out is reached exit */
    do
        {
        HSEStatus = RCC->CR & RCC_CR_HSERDY;
        StartUpCounter++;  
        } while((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

    if ((RCC->CR & RCC_CR_HSERDY) != RESET)
        {
        HSEStatus = (uint32_t)0x01;
        }
    else
        {
        HSEStatus = (uint32_t)0x00;
        }  

    if (HSEStatus == (uint32_t)0x01)
        { 
        /* Enable 64-bit access */
        FLASH->ACR |= FLASH_ACR_ACC64;

        /* Enable Prefetch Buffer */
        FLASH->ACR |= FLASH_ACR_PRFTEN;    

        /* Flash 1 wait state */
        FLASH->ACR |= FLASH_ACR_LATENCY; 

        /* Enable the PWR APB1 Clock */
        RCC->APB1ENR |= RCC_APB1ENR_PWREN;

        /* Select the Voltage Range 1 (1.8V) */
        PWR->CR = PWR_CR_VOS_0;

        /* Wait Until the Voltage Regulator is ready */
        while((PWR->CSR & PWR_CSR_VOSF) != RESET)
            {
            }    

        /* HCLK = SYSCLK */
        RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;
          
        /* PCLK2 = HCLK */
        RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE2_DIV1;

        /* PCLK1 = HCLK */
        RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV1;

        /*  PLL configuration: PLLCLK = (HSE * 6) / 4 = 18MHz */
        RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL | RCC_CFGR_PLLDIV));
        RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PLLMUL6 | RCC_CFGR_PLLDIV4);

        /* Enable PLL */
        RCC->CR |= RCC_CR_PLLON;

        /* Wait till PLL is ready */
        while((RCC->CR & RCC_CR_PLLRDY) == 0)
            {
            }
            
        /* Select PLL as system clock source */
        RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
        RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;    

        /* Wait till PLL is used as system clock source */
        while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)0x0C)
            {
            }
        }
    else
        { 
        /* If HSE fails to start-up, the application will have wrong clock 
           configuration. User can add here some code to deal with this error */    
        /* Go to infinite loop */
        while (1)
            {
            }
        }

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

    /* Configure SysTick to generate an interrupt @1.5KHz (with HCLK = 32MHz) */
    if ( SysTick_Config( 32000000 / 1500 ) )
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