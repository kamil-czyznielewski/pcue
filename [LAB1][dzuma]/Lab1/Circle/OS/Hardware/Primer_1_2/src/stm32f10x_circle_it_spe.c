/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     stm32f10x_circle_it_spe.c
* @brief    Interrupt handler for the CircleOS project.
* @author   FL
* @author   IB
* @date     07/2007
* @version  4.0
* @date     05/2010 Folder reorganization
*
* @note     Platform = Primer1 & 2
*
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/* External variables --------------------------------------------------------*/

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
    GPIO_WriteBit( GPIOA, GPIO_Pin_5, Bit_RESET );
    GPIO_WriteBit( GPIOA, GPIO_Pin_5, Bit_SET );
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

/*******************************************************************************
*
*                                USB_HP_CAN1_TX_IRQHandler
*
*******************************************************************************/
/**
*
*  Default handling for the TX USB IRQ
*
**/
/******************************************************************************/
IRQ void USB_HP_CAN1_TX_IRQHandler( void )
{
#ifdef PRIMER2
    CTR_HP();
#endif
}

/*******************************************************************************
*
*                                USB_LP_CAN1_RX0_IRQHandler
*
*******************************************************************************/
/**
*
*  Default handling for the RX USB IRQ
*
**/
/******************************************************************************/
IRQ void USB_LP_CAN1_RX0_IRQHandler( void )
{
#ifdef PRIMER2
    USB_Istr();
#endif
}

/*******************************************************************************
*
*                                SDIO IRQ Handler
*
*******************************************************************************/
/**
*
*  Default handling for the IRQ-Exception
*
**/
/******************************************************************************/
IRQ void SDIO_IRQHandler( void )
{
#ifdef PRIMER2
    SD_ProcessIRQSrc();
#endif
}

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
    GPIO_WriteBit( GPIOA, GPIO_Pin_7, Bit_RESET );
#endif

    /* Clear TIM2 update interrupt */
    TIM_ClearITPendingBit( TIM2, TIM_IT_Update );

    MEMS_Handler();

#ifdef TIMING_ANALYSIS     // to debug with a scope
    GPIO_WriteBit( GPIOA, GPIO_Pin_7, Bit_SET );
#endif
}

/*******************************************************************************
*
*                                DMA2_Channel2_IRQHandler
*
*******************************************************************************/
/**
*
*  Handles the DMA2 global interrupt request.
*  (Codec in Audio mode : TX from buffer)
*
**/
/******************************************************************************/
IRQ void DMA2_Channel2_IRQHandler( void )
{
#ifdef PRIMER2
    if ( DMA_GetITStatus( DMA2_IT_HT2 ) )
    {
        DMA_ClearITPendingBit( DMA2_IT_GL2 | DMA2_IT_HT2 );
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

    if ( DMA_GetITStatus( DMA2_IT_TC2 ) )
    {
        DMA_ClearITPendingBit( DMA2_IT_GL2 | DMA2_IT_TC2 );
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
#endif //PRIMER2
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
#ifdef PRIMER2
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
#endif //PRIMER2
}




