/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     lcd_spe.h
* @brief    The header file for ILI9325 driver.
* @author   YRT
* @date     08/2010
* @note     Platform = Open4 STM32L Daughter Board
**/
/******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LCD_H
#define __LCD_H

/* Includes ------------------------------------------------------------------*/
#include "IL9325.h"

/* Type def  -----------------------------------------------------------------*/

// Offset between physical and virtual screen
typedef enum
{
    OFFSET_OFF = 0,
    OFFSET_ON = ( 240 - 128 ) / 2
} ENUM_Offset;

/* Data lines configuration mode */
typedef enum
{
    Input,
    Output
} DataConfigMode_TypeDef;

/* Constants -----------------------------------------------------------------*/

//RGB is 16-bit coded as    G2G1G0B4 B3B2B1B0 R4R3R2R1 R0G5G4G3
#define RGB_MAKE(xR,xG,xB)    ( ( (((xG)>>2)&0x07)<<13 ) + ( (((xG)>>2))>>3 )  +      \
                                ( ((xB)>>3) << 8 )          +      \
                                ( ((xR)>>3) << 3 ) )
#define WriteCOM LCD_SendLCDCmd
#define WriteData LCD_SendLCDData

#define WRITE_LCD(addr,val)   { WriteCOM(val); }

/* SCREEN Infos*/
#define PHYS_SCREEN_WIDTH    240
#define PHYS_SCREEN_HEIGHT   320
#define APP_SCREEN_WIDTH     240
#define APP_SCREEN_HEIGHT    240

/* LCD Control pins */

/* LCD addresses as seen by the FSMC (RS = A17) */
#define LCD_DATA_MODE_ADDR   ((u32)0x68020000)
#define LCD_CMD_MODE_ADDR    ((u32)0x68000000)

#define GPIOx_RS_LCD             GPIOB
#define CtrlPin_RS               GPIO_Pin_5
#define GPIO_LCD_RS_PERIPH       RCC_AHBPeriph_GPIOB

#define GPIOx_RD_LCD             GPIOH
#define CtrlPin_RD               GPIO_Pin_2
#define GPIO_LCD_RD_PERIPH       RCC_AHBPeriph_GPIOH

#define GPIOx_WR_LCD             GPIOC
#define CtrlPin_WR               GPIO_Pin_13
#define GPIO_LCD_WR_PERIPH       RCC_AHBPeriph_GPIOC

#define GPIOx_RST_LCD            GPIOA
#define CtrlPin_RST              GPIO_Pin_15
#define GPIO_LCD_RST_PERIPH      RCC_AHBPeriph_GPIOA

#define GPIOx_CS_LCD             GPIOE
#define CtrlPin_CS               GPIO_Pin_11
#define GPIO_LCD_CS_PERIPH       RCC_AHBPeriph_GPIOE

#define GPIOx_D_LCD              GPIOE
#define GPIO_LCD_D_PERIPH        RCC_AHBPeriph_GPIOE
#define LCD_D0                   GPIO_Pin_0
#define LCD_D1                   GPIO_Pin_1
#define LCD_D2                   GPIO_Pin_2
#define LCD_D3                   GPIO_Pin_3
#define LCD_D4                   GPIO_Pin_4
#define LCD_D5                   GPIO_Pin_5
#define LCD_D6                   GPIO_Pin_6
#define LCD_D7                   GPIO_Pin_7
#define LCD_DATA_PINS            (LCD_D0|LCD_D1|LCD_D2|LCD_D3|LCD_D4|LCD_D5|LCD_D6|LCD_D7)

#define GPIOx_BL_LCD             GPIOB
#define GPIO_BACKLIGHT_PIN       GPIO_Pin_4
#define GPIO_LCD_BL_PERIPH       RCC_AHBPeriph_GPIOB
#define GPIO_BL_SOURCE_PIN       GPIO_PinSource4
#define TIM_LCD_BL_PERIPH        RCC_APB1Periph_TIM3
#define TIM_LCD_BL               TIM3
#define TIM_OCxInit              TIM_OC1Init
#define TIM_OCxPreloadConfig     TIM_OC1PreloadConfig
#define GPIOx_BL_LCD_AF          GPIO_AF_TIM3

// Configuration Display Control1 register for orientation :
// TRI=0/DFM=0 (2x8 bits), BGR=1 (no RGB swap), HWM=0 (no high speed), ORG=0
#define V9_MADCTRVAL                0x1000     // Left orientation value   ID=00  AM=0
#define V12_MADCTRVAL               0x1028     // Up orientation value     ID=10, AM=1
#define V3_MADCTRVAL                0x1030     // Right orientation value  ID=11, AM=0
#define V6_MADCTRVAL                0x1018     // Bottom orientation value ID=01, AM=1

#define V12BMP_MADCTRVAL            0x1020     // Up orientation value     ID=10, AM=0
#define V3BMP_MADCTRVAL             0x1038     // Right orientation value  ID=11, AM=1
#define V6BMP_MADCTRVAL             0x1010     // Bottom orientation value ID=01, AM=0
#define V9BMP_MADCTRVAL             0x1008     // Left orientation value   ID=00, AM=1

#endif /*__LCD_H */
