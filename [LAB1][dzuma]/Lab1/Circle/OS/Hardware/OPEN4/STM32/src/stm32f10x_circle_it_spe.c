/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     stm32f10x_circle_it_spe.c
* @brief    Interrupt handler for the CircleOS project.
* @author   FL
* @author   IB
* @date     07/2007
* @version  4.0
* @date     08/2010
* @note     Platform = Open4 STM32C & STM32E
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/* External variables --------------------------------------------------------*/
extern AUDIO_DeviceMode_enum AUDIO_DeviceMode;

/*******************************************************************************
*
*                                NMI_Handler
*
*******************************************************************************/
/**
*
*  Handles the NMI exception.
*
**/
/******************************************************************************/
IRQ void NMI_Handler( void ) {}

/*******************************************************************************
*
*                                HardFault_Handler
*
*******************************************************************************/
/**
*
*  Handles the Hard Fault exception.
*
**/
/******************************************************************************/
IRQ void HardFault_Handler( void )
{
#ifdef TIMING_ANALYSIS     // to debug with a scope
    GPIO_WriteBit( GPIOA, GPIO_Pin_1, Bit_RESET );
    GPIO_WriteBit( GPIOA, GPIO_Pin_1, Bit_SET );
#endif
}

/*******************************************************************************
*
*                                MemManage_Handler
*
*******************************************************************************/
/**
*
*  Handles the Memory Manage exception.
*
**/
/******************************************************************************/
IRQ void MemManage_Handler( void ) {}

/*******************************************************************************
*
*                                BusFault_Handler
*
*******************************************************************************/
/**
*
*  Handles the Bus Fault exception.
*
**/
/******************************************************************************/
IRQ void BusFault_Handler( void ) {}

/*******************************************************************************
*
*                                UsageFault_Handler
*
*******************************************************************************/
/**
*
*  Handles the Usage Fault exception.
*
**/
/******************************************************************************/
IRQ void UsageFault_Handler( void ) {}

/*******************************************************************************
*
*                                DebugMon_Handler
*
*******************************************************************************/
/**
*
*  Handles the  Debug Monitor exception.
*
**/
/******************************************************************************/
IRQ void DebugMon_Handler( void ) {}

/*******************************************************************************
*
*                                SVC_Handler
*
*******************************************************************************/
/**
*
*  Handles the SVCall exception.
*
**/
/******************************************************************************/
IRQ void SVC_Handler( void ) {}

/*******************************************************************************
*
*                                PendSV_Handler
*
*******************************************************************************/
/**
*
*  Handles the PendSVC exception.
*
**/
/******************************************************************************/
IRQ void PendSV_Handler( void ) {}

#ifndef STM32F10X_CL
/*******************************************************************************
* Function Name  : USB_HP_CAN1_TX_IRQHandler
* Description    : This function handles USB High Priority or CAN TX interrupts requests
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
IRQ void USB_HP_CAN1_TX_IRQHandler( void )
{
    CTR_HP();
}

/*******************************************************************************
* Function Name  : USB_LP_CAN1_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
IRQ void USB_LP_CAN1_RX0_IRQHandler( void )
{
    USB_Istr();
}
#endif /* STM32F10X_CL */

#ifdef STM32F10X_HD
/*******************************************************************************
* Function Name  : SDIO_IRQHandler
* Description    : This function handles SDIO global interrupt request.
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
IRQ void SDIO_IRQHandler( void )
{
    /* Process All SDIO Interrupt Sources */
#if SDCARD_AVAIL
    SD_ProcessIRQSrc();
#endif

}
#endif /* STM32F10X_HD */

#ifdef STM32F10X_CL
/*******************************************************************************
* Function Name  : OTG_FS_IRQHandler
* Description    : This function handles USB-On-The-Go FS global interrupt request.
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
IRQ void OTG_FS_IRQHandler( void )
{
    STM32_PCD_OTG_ISR_Handler();
}
#endif /* STM32F10X_CL */

/*******************************************************************************
*
*                                TIM2_IRQHandler
*
*******************************************************************************/
/**
*
*  Handles the TIM2 global interrupt request.
*
**/
/******************************************************************************/
IRQ void TIM2_IRQHandler( void )
{
#ifdef TIMING_ANALYSIS     // to debug with a scope
    GPIO_WriteBit( GPIOA, GPIO_Pin_0, Bit_RESET );
#endif

    /* Clear TIM2 update interrupt */
    TIM_ClearITPendingBit( TIM2, TIM_IT_Update );

    MEMS_Handler();

#ifdef TIMING_ANALYSIS     // to debug with a scope
    GPIO_WriteBit( GPIOA, GPIO_Pin_0, Bit_SET );
#endif
}

/*******************************************************************************
*
*                                DMA1_Channel4_IRQHandler
*
*******************************************************************************/
/**
*
*  Handles the DMA1 global interrupt request.
*  (Codec in Voice mode : RX from I2S-2)
*
**/
/******************************************************************************/
IRQ void DMA1_Channel4_IRQHandler( void )
{
    if ( DMA_GetITStatus( DMA1_IT_HT4 ) )
    {
        DMA_ClearITPendingBit( DMA1_IT_GL4 | DMA1_IT_HT4 );
        voice_record_buffer_fill |= LOW_FULL;
    }
    if ( DMA_GetITStatus( DMA1_IT_TC4 ) )
    {
        DMA_ClearITPendingBit( DMA1_IT_GL4 | DMA1_IT_TC4 );
        voice_record_buffer_fill |= HIGH_FULL;
    }
}

/*******************************************************************************
*
*                                DMA1_Channel5_IRQHandler
*
*******************************************************************************/
/**
*
*  Handles the DMA1 global interrupt request.
*  (Codec in Audio mode : TX from I2S-2)
*
**/
/******************************************************************************/
IRQ void DMA1_Channel5_IRQHandler( void )
{
    if ( DMA_GetITStatus( DMA1_IT_HT5 ) )
    {
        DMA_ClearITPendingBit( DMA1_IT_GL5 | DMA1_IT_HT5 );
        bufferstatus_local |= LOW_EMPTY;
        if ( AUDIO_Format == MONO )
        {
            AUDIO_Cpy_Mono();
        }
        else
        {
            audio_buffer_fill |= LOW_EMPTY;
        }
    }

    if ( DMA_GetITStatus( DMA1_IT_TC5 ) )
    {
        DMA_ClearITPendingBit( DMA1_IT_GL5 | DMA1_IT_TC5 );
        bufferstatus_local |= HIGH_EMPTY;
        if ( AUDIO_Format == MONO )
        {
            AUDIO_Cpy_Mono();
        }
        else
        {
            audio_buffer_fill |= HIGH_EMPTY;
        }
        //  Rq : in mono mode, stop managed by AUDIO_Cpy_Mono()
        //       else it must be managed by the application
    }
}

