/******************** (C) COPYRIGHT 2009 STMicroelectronics ********************
* File Name          : hw_config.c
* Author             : MCD Application Team + Raisonance
* Version            : V3.1.0
* Date               : 10/30/2009
* Description        : Hardware Configuration & Setup for Mass storage USB
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"
#include "ms_hw_config.h"
#if SDCARD_SDIO
#include "sdcard.h"
#endif /* STM32F10X_HD */
#include "mass_mal.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "usb_lib.h"
#include "dosfs.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
ErrorStatus HSEStartUpStatus;

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void RCC_Config( void );
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : Set_System
* Description    : Configures Main system clocks & power
* Input          : None.
* Return         : None.
*******************************************************************************/
NODEBUG void USB_SetSystem( void )
{
    /* RCC configuration */
    UTIL_SetPll(SPEED_VERY_HIGH); 

    /* Enable and Disconnect Line GPIO clock */
    USB_Disconnect_Config();

    /* MAL configuration */
    USB_MALConfig();
}

/*******************************************************************************
* Function Name  : Set_USBClock
* Description    : Configures USB Clock input (48MHz)
* Input          : None.
* Return         : None.
*******************************************************************************/
NODEBUG void USB_SetClock( void )
{
#ifdef STM32F10X_CL     // STM32C
  /* Select USBCLK source */
  RCC_OTGFSCLKConfig(RCC_OTGFSCLKSource_PLLVCO_Div3);

  /* Enable the USB clock */ 
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_OTG_FS, ENABLE) ;
#endif

#ifdef STM32F2XX        // STM3220G
  /* Enable the USB clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_OTG_HS, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_OTG_HS_ULPI, ENABLE);
#endif

#ifdef STM32L1XX_MD     // STM32L
  /* Enable the USB clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
#endif

#ifdef STM32F10X_HD     // Primer2 & STM32E
  /* Select USBCLK source */
  RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5); 

  /* Enable the USB clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
#endif

}

/*******************************************************************************
* Function Name  : Enter_LowPowerMode
* Description    : Power-off system clocks and power while entering suspend mode
* Input          : None.
* Return         : None.
*******************************************************************************/
NODEBUG void USB_Enter_LowPowerMode( void )
{
    /* Set the device state to suspend */
    bDeviceState = SUSPENDED;
}

/*******************************************************************************
* Function Name  : Leave_LowPowerMode
* Description    : Restores system clocks and power while exiting suspend mode
* Input          : None.
* Return         : None.
*******************************************************************************/
NODEBUG void USB_Leave_LowPowerMode( void )
{
    DEVICE_INFO* pInfo = &Device_Info;

    /* Set the device state to the correct state */
    if ( pInfo->Current_Configuration != 0 )
    {
        /* Device configured */
        bDeviceState = CONFIGURED;
    }
    else
    {
        bDeviceState = ATTACHED;
    }

}

/*******************************************************************************
* Function Name  : USB_Interrupts_Config
* Description    : Configures the USB interrupts
* Input          : None.
* Return         : None.
*******************************************************************************/
NODEBUG void USB_Interrupts_Config( void )
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

#ifdef STM32F10X_CL     // STM32C
    NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);       
#endif
    
#ifdef STM32F2XX        // STM3220G
    NVIC_InitStructure.NVIC_IRQChannel = OTG_HS_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);       
#endif
    
#ifdef STM32L1XX_MD     // STM32L
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USB_HP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif

#ifdef STM32F10X_HD     // Primer2 & STM32E
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif
    
#if SDCARD_SDIO  
    NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);
#endif // SDCARD_SDIO
  }

/*******************************************************************************
* Function Name  : USB_Interrupts_Stop.
* Description    : Stop the USB interrupts.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
NODEBUG void USB_Interrupts_Stop( void )
{
   NVIC_InitTypeDef NVIC_InitStructure;
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

#ifdef STM32F10X_CL     // STM32C
    NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Stop USB
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, DISABLE);  
#endif

#ifdef STM32F2XX        // STM3220G
    NVIC_InitStructure.NVIC_IRQChannel = OTG_HS_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Stop USB
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_OTG_HS, DISABLE);  
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_OTG_HS_ULPI, DISABLE); 
#endif

#ifdef STM32L1XX_MD     // STM32L
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //FL071018 0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USB_HP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //FL071018 0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // Stop USB
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, DISABLE);  
#endif    

#ifdef STM32F10X_HD     // Primer2 & STM32E 
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //FL071018 0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //FL071018 0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // Stop USB
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, DISABLE);  
#endif

}


/*******************************************************************************
* Function Name  : USB_Cable_Config
* Description    : Software Connection/Disconnection of USB Cable.
* Input          : None.
* Return         : Status
*******************************************************************************/
NODEBUG void USB_Cable_Config( FunctionalState NewState )
{
#ifdef STM32F10X_CL
    if ( NewState != DISABLE )
    {
        USB_DevConnect();
    }
    else
    {
        USB_DevDisconnect();
    }
#endif /* STM32F10X_CL */

    if ( NewState != DISABLE )
    {
        GPIO_ResetBits( USB_DISCONNECT_PORT, USB_DISCONNECT_PIN );
    }
    else
    {
        GPIO_SetBits( USB_DISCONNECT_PORT, USB_DISCONNECT_PIN );
    }

}

/*******************************************************************************
* Function Name  : Get_SerialNum.
* Description    : Create the serial number string descriptor.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
NODEBUG void USB_GetSerialNum( void )
{
    uint32_t Device_Serial0, Device_Serial1, Device_Serial2;

#ifndef STM32L1XX_MD
    Device_Serial0 = *( __IO uint32_t* )( 0x1FFFF7E8 );
    Device_Serial1 = *( __IO uint32_t* )( 0x1FFFF7EC );
    Device_Serial2 = *( __IO uint32_t* )( 0x1FFFF7F0 );
#else
    Device_Serial0 = *( __IO uint32_t* )( 0x1FF007E8 );
    Device_Serial1 = *( __IO uint32_t* )( 0x1FF007EC );
    Device_Serial2 = *( __IO uint32_t* )( 0x1FF007F0 );
#endif

    if ( Device_Serial0 != 0 )
    {
        MASS_StringSerial[2] = ( uint8_t )( Device_Serial0 & 0x000000FF );
        MASS_StringSerial[4] = ( uint8_t )(( Device_Serial0 & 0x0000FF00 ) >> 8 );
        MASS_StringSerial[6] = ( uint8_t )(( Device_Serial0 & 0x00FF0000 ) >> 16 );
        MASS_StringSerial[8] = ( uint8_t )(( Device_Serial0 & 0xFF000000 ) >> 24 );

        MASS_StringSerial[10] = ( uint8_t )( Device_Serial1 & 0x000000FF );
        MASS_StringSerial[12] = ( uint8_t )(( Device_Serial1 & 0x0000FF00 ) >> 8 );
        MASS_StringSerial[14] = ( uint8_t )(( Device_Serial1 & 0x00FF0000 ) >> 16 );
        MASS_StringSerial[16] = ( uint8_t )(( Device_Serial1 & 0xFF000000 ) >> 24 );

        MASS_StringSerial[18] = ( uint8_t )( Device_Serial2 & 0x000000FF );
        MASS_StringSerial[20] = ( uint8_t )(( Device_Serial2 & 0x0000FF00 ) >> 8 );
        MASS_StringSerial[22] = ( uint8_t )(( Device_Serial2 & 0x00FF0000 ) >> 16 );
        MASS_StringSerial[24] = ( uint8_t )(( Device_Serial2 & 0xFF000000 ) >> 24 );
    }
}

/*******************************************************************************
* Function Name  : MAL_Config
* Description    : MAL_layer configuration
* Input          : None.
* Return         : None.
*******************************************************************************/
NODEBUG void USB_MALConfig( void )
{
    MAL_Init( 0 );

#ifdef STM32F10X_HD
    /* Enable the FSMC Clock */
    RCC_AHBPeriphClockCmd( RCC_AHBPeriph_FSMC, ENABLE );
    MAL_Init( 1 );
#endif /* STM32F10X_HD */
}

/*******************************************************************************
* Function Name  : USB_Disconnect_Config
* Description    : Disconnect pin configuration
* Input          : None.
* Return         : None.
*******************************************************************************/
NODEBUG void USB_Disconnect_Config( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = USB_DISCONNECT_PIN;

#ifdef STM32L1XX_MD
    /* Enable USB GPIO clock */
    RCC_AHBPeriphClockCmd(RCC_APBxPeriph_GPIO_DISCONNECT, ENABLE);
    
    /* USB_DISCONNECT_PIN used as USB pull-up */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

#else

    #ifdef STM32F2XX
    /* Enable USB GPIO clock */
    RCC_AHB1PeriphClockCmd(RCC_APBxPeriph_GPIO_DISCONNECT, ENABLE);
    
    /* USB_DISCONNECT_PIN used as USB pull-up */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    
    #else

    /* Enable USB GPIO clock */
    RCC_APB2PeriphClockCmd(RCC_APBxPeriph_GPIO_DISCONNECT, ENABLE);
    
    /* USB_DISCONNECT_PIN used as USB pull-up */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    
    #endif // STM32F2XX
    
#endif // STM32L1XX_MD

    GPIO_Init(USB_DISCONNECT_PORT, &GPIO_InitStructure);
    
#ifdef STM32F10X_CL
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_PWRON, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_OVER, ENABLE);

    /* USB POWER ON PIN used as push pull */
    GPIO_InitStructure.GPIO_Pin = USB_PWRON_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(USB_PWRON_PORT, &GPIO_InitStructure);
    GPIO_SetBits(USB_PWRON_PORT, USB_PWRON_PIN);

    /* USB OVERCURRENT used as input */
    GPIO_InitStructure.GPIO_Pin = USB_OVER_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(USB_OVER_PORT, &GPIO_InitStructure);
#endif
}

/* Public functions ---------------------------------------------------------*/

/*******************************************************************************
*
*                                Fct_Download_Ini
*
*******************************************************************************/
/**
*  Initialization of the SDCard in Mass storage mode.
*
*  @retval  MENU_CONTINUE_COMMAND.
*
**/
/******************************************************************************/
NODEBUG2 enum MENU_code Fct_Download_Ini( void )
{
#if LCD_HIGH_DEF
    static const u16 bmpComputer [] =
    {
#include "bmp\Computer-48x48.h"
    };
#define COMPUTER_ICON_SIZE 48
#else
    static const u16 bmpComputer [] =
    {
#include "bmp\Computer-32x32.h"
#define COMPUTER_ICON_SIZE 32
    };
#endif

    // Presence SDCard test
    if ( FS_Mount( MMCSD_SDIO ) == 0xFFFFFFFF )
    {
        MENU_Print( "No SDCARD" );
    }
    else
    {
        /* Disable TIM2 (MEMS) interrupt */
        BUZZER_SetMode( BUZZER_OFF );
        TIM_ITConfig( TIM2, TIM_IT_Update, DISABLE );

        USB_SetSystem();
        USB_SetClock();
        USB_Interrupts_Config();
        USB_Init();

        LCD_FillRect( 0, 0, Screen_Width, Screen_Height, RGB_WHITE );

        // Character magnify deponding on the platform !
        DRAW_RestoreCharMagniCoeff();

#if LCD_HIGH_DEF
        DRAW_SetImage( bmpComputer, ( Screen_Width - COMPUTER_ICON_SIZE ) / 2, 170, COMPUTER_ICON_SIZE, COMPUTER_ICON_SIZE );

        DRAW_SetCursorPos( 5, 120 );
        DRAW_Puts( "Connect USB to\n" );
        DRAW_Puts( "PC and navigate.\n\n" );
        DRAW_Puts( "Push button when\nfinished..." );
#else
        DRAW_SetImage( bmpComputer, ( Screen_Width - COMPUTER_ICON_SIZE ) / 2, 80, COMPUTER_ICON_SIZE, COMPUTER_ICON_SIZE );

        DRAW_SetCursorPos( 5, 60 );
        DRAW_Puts( "Connect USB to PC\n" );
        DRAW_Puts( "and navigate.\n\n" );
        DRAW_Puts( "Push button when\nfinished..." );
#endif
        DRAW_SetCharMagniCoeff( 1 );
    }

    return MENU_CONTINUE_COMMAND;
}

/*******************************************************************************
*
*                                Fct_Download_Handler
*
*******************************************************************************/
/**
*  Wait for the end of mass storage mode by user.
*
*  @retval  MENU_CONTINUE or MENU_LEAVE when finished.
*
**/
/******************************************************************************/
NODEBUG2 enum MENU_code Fct_Download_Handler( void )
{

    // When the user pushed the button, it's time to leave!
    if ( BUTTON_GetState() == BUTTON_PUSHED_FORMAIN )
    {
        //Stop the USB
        USB_Cable_Config( DISABLE );

        //Stop the USB IRQ
        USB_Interrupts_Stop() ;

        /* Restore TIM2 (MEMS) interrupt */
        TIM_ITConfig( TIM2, TIM_IT_Update, ENABLE );

        return fQuit();
    }
    return MENU_CONTINUE;
}


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
