/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     adc_spe.c
* @brief    ADC initialization.
* @author   FL
* @date     07/2007
*
* @version  4.0
* @date     08/2010 Add Open4
* @note     Platform = Open4 STM32L
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private defines ---------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
ADC_InitTypeDef   ADC_InitStructure;
ADC_CommonInitTypeDef ADC_CommonInitStructure;
DMA_InitTypeDef   DMA_InitStructure;

u16               ADC_ConvertedValue[ADC_DMA_SIZE];

/* Private functions ---------------------------------------------------------*/


/*******************************************************************************
*
*                                ADConverter_Init
*
*******************************************************************************/
/**
*
*  ADC initialization (to measure the battery voltage, the temperature
*  and manage the touchscreen)
*
**/
/******************************************************************************/
void ADConverter_Init( void )
    {

    /* Enable DMA clock */
    RCC_AHBPeriphClockCmd( RCC_AHBPeriph_DMA1, ENABLE );

    /* Configure Touch Screen inputs as analog input ---------------*/
    RCC_PERIPH_GPIO_CLOCK_CMD( RCC_APBxPeriph_GPIOx_TEMP | RCC_APBxPeriph_GPIOx_TS_R 
                             | RCC_APBxPeriph_GPIOx_TS_U | RCC_APBxPeriph_GPIOx_TS_D | RCC_APBxPeriph_GPIOx_TS_L, ENABLE );

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_TOUCH_R;
    GPIO_Init( GPIOx_TOUCH_R, &GPIO_InitStructure );

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_TOUCH_U;
    GPIO_Init( GPIOx_TOUCH_U, &GPIO_InitStructure );
    
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_TOUCH_D;
    GPIO_Init( GPIOx_TOUCH_D, &GPIO_InitStructure );
    
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_TOUCH_L;
    GPIO_Init( GPIOx_TOUCH_L, &GPIO_InitStructure );
    
    /* Configure VBat input as analog input ---------------*/
    GPIO_InitStructure.GPIO_Pin = GPIO_VBAT_PIN;
    GPIO_Init( GPIOx_VBAT, &GPIO_InitStructure );

    /* Configure Temperature sensor input as analog input ---------------*/
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_TEMP_SENSOR;
    GPIO_Init( GPIOx_TEMP_SENSOR, &GPIO_InitStructure );

    /* Enable ADC1 clock */
    RCC_HSICmd(ENABLE);             /* ADCCLK = HSI */
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1, ENABLE );

    /*
    * Configure ADC
    *
    */

    /* DMA Channel 1 Configuration ----------------------------------------------*/
    DMA_DeInit( DMA1_Channel1 );

    DMA_InitStructure.DMA_PeripheralBaseAddr  = ADC1_DR_ADDRESS;
    DMA_InitStructure.DMA_MemoryBaseAddr      = ( u32 )&ADC_ConvertedValue[0];
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize          = ADC_DMA_SIZE;
    DMA_InitStructure.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc           = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize      = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode                = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority            = DMA_Priority_Low;
    DMA_InitStructure.DMA_M2M                 = DMA_M2M_Disable;
    DMA_Init( DMA1_Channel1, &DMA_InitStructure );

    /* Enable DMA Channel 1 */
    DMA_Cmd( DMA1_Channel1, ENABLE );

    /* ADC1 Configuration ------------------------------------------------------*/
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;  // Clock = HSI / 4 = 4 MHz
    ADC_CommonInit(&ADC_CommonInitStructure);
    
    ADC_InitStructure.ADC_Resolution           = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode         = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode   = ENABLE;
//    ADC_InitStructure.ADC_ExternalTrigConv     = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_DataAlign            = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion      = 6;         // Nb Regular channels
    ADC_Init( ADC1, &ADC_InitStructure );

    ADC_TempSensorVrefintCmd( ENABLE ); /*%%*/

    /* ADC1 Regular Channel 6 Configuration = Vbatt */
    ADC_RegularChannelConfig( ADC1, ADC_Channel_6,  1, ADC_SampleTime_384Cycles );

    /* ADC1 Regular Channel 7 Configuration = External temperature sensor */
    ADC_RegularChannelConfig( ADC1, ADC_Channel_7,  2, ADC_SampleTime_384Cycles );

    /* ADC1 Regular Channel 16 internal temperature */
//    ADC_RegularChannelConfig( ADC1, ADC_Channel_16, 2, ADC_SampleTime_48Cycles );

    /* ADC1 Regular Channel 8 to 11 Configuration = touchscreen */
    ADC_RegularChannelConfig( ADC1, ADC_Channel_11, 3, ADC_SampleTime_384Cycles ); // TouchScreen L 
    ADC_RegularChannelConfig( ADC1, ADC_Channel_9,  4, ADC_SampleTime_384Cycles ); // TouchScreen U
    ADC_RegularChannelConfig( ADC1, ADC_Channel_10, 5, ADC_SampleTime_384Cycles ); // TouchScreen D

    /* ADC1 Regular Channel 17 = internal Vref */
    ADC_RegularChannelConfig( ADC1, ADC_Channel_17, 6, ADC_SampleTime_48Cycles );

    /* Enable ADC1  */
    ADC_Cmd( ADC1, ENABLE );

    /* Enable Vrefint channel 17 */
    ADC_TempSensorVrefintCmd( ENABLE );

    /* Enable the request after last transfer for DMA Circular mode */
    ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

    /* Enable ADC1 DMA */
    ADC_DMACmd(ADC1, ENABLE);

    /* Enable ADC1 */
    ADC_Cmd(ADC1, ENABLE);

    /* Start ADC1 Software Conversion */ 
    ADC_SoftwareStartConv(ADC1);
    
    }

/// @endcond
