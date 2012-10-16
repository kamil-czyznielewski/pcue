/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     adc_spe.c
* @brief    ADC initialization.
* @author   FL
* @date     07/2007
* @version  3.0
* @date     11/2008 Add Primer 2 and ST library v2.0.3
* @version  4.0
* @date     05/2010 Folder reorganization
*
* @note     Platform = Primer1 & 2
*
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private defines ---------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
ADC_InitTypeDef   ADC_InitStructure;
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
NODEBUG void ADConverter_Init( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable ADC1 and Vbatclock */
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOB, ENABLE );

    /* Enable DMA clocks */
    RCC_AHBPeriphClockCmd( RCC_AHBPeriph_DMA1, ENABLE );

#ifdef PRIMER2
    /* Enable Touchscreen clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE );

    /* Configure PC.01 to PC.03 (ADC Channel10-13) as analog input ---------------*/
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init( GPIOC, &GPIO_InitStructure );
#endif

    /* Configure VBat input as analog input ---------------*/
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init( GPIOB, &GPIO_InitStructure );

    /*
    * Configure ADC
    *
    */

    /* DMA Channel 1 Configuration ----------------------------------------------*/
    DMA_DeInit( DMA1_Channel1 );

    DMA_InitStructure.DMA_PeripheralBaseAddr  = ADC1_DR_Address;
    DMA_InitStructure.DMA_MemoryBaseAddr      = ( u32 )&ADC_ConvertedValue[0];
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_PeripheralSRC;
    /*__DMA_InitStructure.DMA_BufferSize          = 1+4+4;//2;//%% 1;   // YRT20081010 Touschscreen added*/
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
    ADC_InitStructure.ADC_Mode                = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode        = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode  = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv    = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign           = ADC_DataAlign_Right;
    /*__YRT   ADC_InitStructure.ADC_NbrOfChannel        = 1+4+4; // YRT20081010 Touschscreen added*/
    ADC_InitStructure.ADC_NbrOfChannel        = ADC_NB_CHANNELS;

    ADC_Init( ADC1, &ADC_InitStructure );

    ADC_TempSensorVrefintCmd( ENABLE ); /*%%*/

    /* ADC1 Regular Channel 9 Configuration = Vbatt */
    ADC_RegularChannelConfig( ADC1, ADC_Channel_9,  1, ADC_SampleTime_239Cycles5 );

    /* ADC1 Regular Channel 16 internal temperature */
    ADC_RegularChannelConfig( ADC1, ADC_Channel_16, 2, ADC_SampleTime_55Cycles5 );

#ifdef PRIMER2
    /* ADC1 Regular Channel 10 to 13 Configuration = touchscreen */
    ADC_RegularChannelConfig( ADC1, ADC_Channel_10, 3, ADC_SampleTime_239Cycles5 );
    ADC_RegularChannelConfig( ADC1, ADC_Channel_12, 4, ADC_SampleTime_239Cycles5 );
    ADC_RegularChannelConfig( ADC1, ADC_Channel_13, 5, ADC_SampleTime_239Cycles5 );
#endif

    /* Enable ADC1 external trigger conversion */
    ADC_ExternalTrigConvCmd( ADC1, ENABLE );  /* <==== moved above cal and above enable in circleOS 1.8(a)*/

    /* Enable ADC1  */
    ADC_Cmd( ADC1, ENABLE );

#if 1                                         /* <=== changed from 0 to 1 in circleOS 1.8(a)*/
    /* Enable Vrefint channel 17                                    -- from ST library examples*/
    ADC_TempSensorVrefintCmd( ENABLE );

    /* Enable ADC1 reset calibaration register */
    ADC_ResetCalibration( ADC1 );

    /* Check the end of ADC1 reset calibration register */
    while ( ADC_GetResetCalibrationStatus( ADC1 ) );

    /* Start ADC1 calibaration */
    ADC_StartCalibration( ADC1 );
    /* Check the end of ADC1 calibration */
    while ( ADC_GetCalibrationStatus( ADC1 ) );                  /*  -- end ST example clip */

#endif

    /* Enable ADC1's DMA interface */
    ADC_DMACmd( ADC1, ENABLE );

    /* Start ADC1 Software Conversion */
    ADC_SoftwareStartConvCmd( ADC1, ENABLE );
}

/// @endcond
