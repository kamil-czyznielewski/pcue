/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     stm32f10x_circle_it_spe.c
* @brief    Interrupt handler for the CircleOS project.
* @author   YRT
* @version  4.0
* @date     08/2010
* @note     Platform = Open4 STM32L
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

/*******************************************************************************
* Function Name  : USB_HP_IRQHandler
* Description    : This function handles USB High Priority or CAN TX interrupts requests
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
IRQ void USB_HP_IRQHandler( void )
{
    CTR_HP();
}

/*******************************************************************************
* Function Name  : USB_LP_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
IRQ void USB_LP_IRQHandler( void )
{
    USB_Istr();
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
    GPIO_WriteBit( GPIOA, GPIO_Pin_4, Bit_RESET );
#endif

    /* Clear TIM2 update interrupt */
    TIM_ClearITPendingBit( TIM2, TIM_IT_Update );

    MEMS_Handler();

#ifdef TIMING_ANALYSIS     // to debug with a scope
    GPIO_WriteBit( GPIOA, GPIO_Pin_4, Bit_SET );
#endif
}



