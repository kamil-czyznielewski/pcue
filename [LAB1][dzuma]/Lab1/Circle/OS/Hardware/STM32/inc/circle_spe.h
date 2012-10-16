/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     Circle_spe.h
* @brief    STM32 platform specific declarations.
* @author   YRT
* @date     05/2010
* @version  4.0
*
* @attention Contains the same defines for all platform
**/
/******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CIRSPE_H
#define __CIRSPE_H

/* Primers 1 & 2 -------------------------*/
#ifdef PRIMER1
#define SCREEN_WIDTH        128         /*< Default application Width of visible screen in pixels. */
#define SCREEN_HEIGHT       128         /*< Default application Height of visible screen in pixels. */
#define CHIP_SCREEN_WIDTH   128         /*< Width of screen driven by LCD controller in pixels. */
#define CHIP_SCREEN_HEIGHT  128         /*< Height of screen driven by LCD controller in pixels. */
typedef enum                            /* Offset between physical and virtual screen   */
{
    OFFSET_OFF = 0,
    OFFSET_ON = 0
} ENUM_Offset;
#endif

#ifdef PRIMER2
#define BUTTON_HEIGHT       32          /*< Height of toolbar button.  */
#define BUTTON_WIDTH        32          /*< Width of toolbar button.  */
#define ICON_HEIGHT         32          /*< Height of toolbar icon.  */
#define ICON_WIDTH          32          /*< Width of toolbar icon.  */
#define SCREEN_WIDTH        128         /*< Default application Width of visible screen in pixels.   */
#define SCREEN_HEIGHT       128         /*< Default application Height of visible screen in pixels.  */
#define CHIP_SCREEN_WIDTH   132         /*< Width of screen driven by LCD controller in pixels.  */
#define CHIP_SCREEN_HEIGHT  132         /*< Height of screen driven by LCD controller in pixels.  */
typedef enum                            /* Offset between physical and virtual screen   */
{
    OFFSET_OFF = 0,
    OFFSET_ON = 0
} ENUM_Offset;
#endif

/* Open4 (STM32E/STM32C...)---------------*/
#ifdef OPEN4
#define BUTTON_HEIGHT       80          /*< Height of toolbar button.  */
#define BUTTON_WIDTH        60          /*< Width of toolbar button.  */
#define ICON_HEIGHT         60          /*< Height of toolbar icon.  */
#define ICON_WIDTH          60          /*< Width of toolbar icon.  */
#define SCREEN_WIDTH        240         /*< Default application Width of visible screen in pixels.   */
#define SCREEN_HEIGHT       240         /*< Default application Height of visible screen in pixels.  */
#define CHIP_SCREEN_WIDTH   240         /*< Width of screen driven by LCD controller in pixels.  */
#define CHIP_SCREEN_HEIGHT  320         /*< Height of screen driven by LCD controller in pixels.  */
typedef enum                            /* Offset between physical and virtual screen   */
{
    OFFSET_OFF = 0,
    OFFSET_ON = ( 240 - 128 ) / 2
} ENUM_Offset;
#endif

#endif /*__CIRSPE_H */
