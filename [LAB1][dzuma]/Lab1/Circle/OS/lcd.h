/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     lcd.h
* @brief    The header file for ST7637 driver.
* @author   IB
* @date     07/2007
*
**/
/******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LCD_H
#define __LCD_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "circle.h"

/* Type def  -----------------------------------------------------------------*/

/* Data lines configuration mode */
typedef enum
{
    Input,
    Output
} DataConfigMode_TypeDef;

/* Constants -----------------------------------------------------------------*/

/* LCD Control pins */

#ifdef PRIMER1

#define TIMER_IT_CHANN           TIM_IT_CC2
#define CtrlPin_RS               GPIO_Pin_8
#define CtrlPin_RD               GPIO_Pin_9
#define CtrlPin_WR               GPIO_Pin_10
#define CtrlPin_RST              GPIO_Pin_12
#define LCD_CTRL_PINS            (CtrlPin_RS|CtrlPin_RD|CtrlPin_WR|CtrlPin_RST)
#define GPIOx_CTRL_LCD           GPIOC
#define GPIO_LCD_CTRL_PERIPH     RCC_APB2Periph_GPIOC

#define CtrlPin_CS               GPIO_Pin_11
#define GPIOx_CS_LCD             GPIOC
#define GPIO_LCD_CS_PERIPH       RCC_APB2Periph_GPIOC

#define LCD_D0                   GPIO_Pin_0
#define LCD_D1                   GPIO_Pin_1
#define LCD_D2                   GPIO_Pin_2
#define LCD_D3                   GPIO_Pin_3
#define LCD_D4                   GPIO_Pin_4
#define LCD_D5                   GPIO_Pin_5
#define LCD_D6                   GPIO_Pin_6
#define LCD_D7                   GPIO_Pin_7
#define LCD_DATA_PINS            (LCD_D0|LCD_D1|LCD_D2|LCD_D3|LCD_D4|LCD_D5|LCD_D6|LCD_D7)
#define GPIOx_D_LCD              GPIOC
#define GPIO_LCD_D_PERIPH        RCC_APB2Periph_GPIOC
#define GPIO_BACKLIGHT_PIN       GPIO_Pin_7


/* LCD Commands */
#define DISPLAY_ON               0xAF
#define DISPLAY_OFF              0xAE
#define START_LINE               0xC0
#define START_COLUMN             0x00
#define CLOCKWISE_OUTPUT         0xA0
#define DYNAMIC_DRIVE            0xA4
#define DUTY_CYCLE               0xA9
#define READ_MODIFY_WRITE_OFF    0xEE
#define SOFTWARE_RESET           0xE2

#define ST7637_NOP               0x00
#define ST7637_SWRESET           0x01
#define ST7637_RDDID             0x04
#define ST7637_RDDST             0x09
#define ST7637_RDDPM             0x0A
#define ST7637_RDDMADCTR         0x0B
#define ST7637_RDDCOLMOD         0x0C
#define ST7637_RDDIM             0x0D
#define ST7637_RDDSM             0x0E
#define ST7637_RDDSDR            0x0F

#define ST7637_SLPIN             0x10
#define ST7637_SLPOUT            0x11
#define ST7637_PTLON             0x12
#define ST7637_NORON             0x13

#define ST7637_INVOFF            0x20
#define ST7637_INVON             0x21
#define ST7637_APOFF             0x22
#define ST7637_APON              0x23
#define ST7637_WRCNTR            0x25
#define ST7637_DISPOFF           0x28
#define ST7637_DISPON            0x29
#define ST7637_CASET             0x2A
#define ST7637_RASET             0x2B
#define ST7637_RAMWR             0x2C
#define ST7637_RGBSET            0x2D
#define ST7637_RAMRD             0x2E

#define ST7637_PTLAR             0x30
#define ST7637_SCRLAR            0x33
#define ST7637_TEOFF             0x34
#define ST7637_TEON              0x35
#define ST7637_MADCTR            0x36
#define ST7637_VSCSAD            0x37
#define ST7637_IDMOFF            0x38
#define ST7637_IDMON             0x39
#define ST7637_COLMOD            0x3A

#define ST7637_RDID1             0xDA
#define ST7637_RDID2             0xDB
#define ST7637_RDID3             0xDC

#define ST7637_DUTYSET           0xB0
#define ST7637_FIRSTCOM          0xB1
#define ST7637_OSCDIV            0xB3
#define ST7637_PTLMOD            0xB4
#define ST7637_NLINVSET          0xB5
#define ST7637_COMSCANDIR        0xB7
#define ST7637_RMWIN             0xB8
#define ST7637_RMWOUT            0xB9

#define ST7637_VOPSET            0xC0
#define ST7637_VOPOFSETINC       0xC1
#define ST7637_VOPOFSETDEC       0xC2
#define ST7637_BIASSEL           0xC3
#define ST7637_BSTBMPXSEL        0xC4
#define ST7637_BSTEFFSEL         0xC5
#define ST7637_VOPOFFSET         0xC7
#define ST7637_VGSORCSEL         0xCB

#define ST7637_ID1SET            0xCC
#define ST7637_ID2SET            0xCD
#define ST7637_ID3SET            0xCE

#define ST7637_ANASET            0xD0
#define ST7637_AUTOLOADSET       0xD7
#define ST7637_RDTSTSTATUS       0xDE

#define ST7637_EPCTIN            0xE0
#define ST7637_EPCTOUT           0xE1
#define ST7637_EPMWR             0xE2
#define ST7637_EPMRD             0xE3
#define ST7637_MTPSEL            0xE4
#define ST7637_ROMSET            0xE5
#define ST7637_HPMSET            0xEB

#define ST7637_FRMSEL            0xF0
#define ST7637_FRM8SEL           0xF1
#define ST7637_TMPRNG            0xF2
#define ST7637_TMPHYS            0xF3
#define ST7637_TEMPSEL           0xF4
#define ST7637_THYS              0xF7
#define ST7637_FRAMESET          0xF9

#define ST7637_MAXCOL            0x83
#define ST7637_MAXPAG            0x83

#endif /*PRIMER1*/



#ifdef PRIMER2

/*/ LCD addresses as seen by the FSMC*/
#define LCD_DATA_MODE_ADDR   ((u32)0x68020000)
#define LCD_CMD_MODE_ADDR    ((u32)0x68000000)

#define TIMER_IT_CHANN           TIM_IT_CC2

#define CtrlPin_RS               GPIO_Pin_11
#define CtrlPin_RD               GPIO_Pin_4
#define CtrlPin_WR               GPIO_Pin_5
#define CtrlPin_RST              GPIO_Pin_6
#define LCD_CTRL_PINS            (CtrlPin_RS|CtrlPin_RD|CtrlPin_WR)
#define GPIOx_CTRL_LCD           GPIOD
#define GPIO_LCD_CTRL_PERIPH     RCC_APB2Periph_GPIOD

#define CtrlPin_CS               GPIO_Pin_7
#define GPIOx_CS_LCD             GPIOD
#define GPIO_LCD_CS_PERIPH       RCC_APB2Periph_GPIOD

#define LCD_D0                   GPIO_Pin_7
#define LCD_D1                   GPIO_Pin_8
#define LCD_D2                   GPIO_Pin_9
#define LCD_D3                   GPIO_Pin_10
#define LCD_D4                   GPIO_Pin_11
#define LCD_D5                   GPIO_Pin_12
#define LCD_D6                   GPIO_Pin_13
#define LCD_D7                   GPIO_Pin_14
#define LCD_DATA_PINS            (LCD_D0|LCD_D1|LCD_D2|LCD_D3|LCD_D4|LCD_D5|LCD_D6|LCD_D7)
#define GPIOx_D_LCD              GPIOE
#define GPIO_LCD_D_PERIPH        RCC_APB2Periph_GPIOE

#define  GPIO_BACKLIGHT_PIN      GPIO_Pin_8

/* LCD Commands */
#define DISPLAY_ON               0xAF
#define DISPLAY_OFF              0xAE
#define START_LINE               0xC0
#define START_COLUMN             0x00
#define CLOCKWISE_OUTPUT         0xA0
#define DYNAMIC_DRIVE            0xA4
#define DUTY_CYCLE               0xA9
#define READ_MODIFY_WRITE_OFF    0xEE
#define SOFTWARE_RESET           0xE2

#define ST7732_NOP               0x00
#define ST7732_SWRESET           0x01
#define ST7732_RDDID             0x04
#define ST7732_RDDST             0x09
#define ST7732_RDDPM             0x0A
#define ST7732_RDDMADCTR         0x0B
#define ST7732_RDDCOLMOD         0x0C
#define ST7732_RDDIM             0x0D
#define ST7732_RDDSM             0x0E
#define ST7732_RDDSDR            0x0F

#define ST7732_SLPIN             0x10
#define ST7732_SLPOUT            0x11
#define ST7732_PTLON             0x12
#define ST7732_NORON             0x13

#define ST7732_INVOFF            0x20
#define ST7732_INVON             0x21
#define ST7732_GAMSET            0x26
#define ST7732_DISPOFF           0x28
#define ST7732_DISPON            0x29
#define ST7732_CASET             0x2A
#define ST7732_RASET             0x2B
#define ST7732_RAMWR             0x2C
#define ST7732_RGBSET            0x2D
#define ST7732_RAMRD             0x2E

#define ST7732_PTLAR             0x30
#define ST7732_SCRLAR            0x33
#define ST7732_TEOFF             0x34
#define ST7732_TEON              0x35
#define ST7732_MADCTR            0x36
#define ST7732_VSCSAD            0x37
#define ST7732_IDMOFF            0x38
#define ST7732_IDMON             0x39
#define ST7732_COLMOD            0x3A

#define ST7732_RDID1             0xDA
#define ST7732_RDID2             0xDB
#define ST7732_RDID3             0xDC

#define ST7732_RGBCTR            0xB0
#define ST7732_FRMCTR1           0xB1
#define ST7732_FRMCTR2           0xB2
#define ST7732_FRMCTR3           0xB3
#define ST7732_INVCTR            0xB4
#define ST7732_RGBBPCTR          0xB5
#define ST7732_DISSET5           0xB6

#define ST7732_PWCTR1            0xC0
#define ST7732_PWCTR2            0xC1
#define ST7732_PWCTR3            0xC2
#define ST7732_PWCTR4            0xC3
#define ST7732_PWCTR5            0xC4
#define ST7732_VMCTR1            0xC5
#define ST7732_VMOFCTR           0xC7

#define ST7732_WRID2             0xD1
#define ST7732_WRID3             0xD2
#define ST7732_RDID4             0xD3
#define ST7732_NVCTR1            0xD9
#define ST7732_NVCTR2            0xDE
#define ST7732_NVCTR3            0xDF

#define ST7732_GAMCTRP1          0xE0
#define ST7732_GAMCTRN1          0xE1

#define ST7732_AUTOCTRL          0xF1
#define ST7732_OSCADJ            0xF2
#define ST7732_DISPCTRL          0xF5
#define ST7732_DEFADJ            0xF6

#endif /*PRIMER2*/


#endif /*__LCD_H */
