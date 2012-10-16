/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     circle_platform.h
* @brief    PRIMER 1 & 2 specific general header for the CircleOS project.
* @author   YRT
* @date     09/2009
* @version  4.0
*
* @Note     Split form circle.h
*
**/
/******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PLATFORM_H
#define __PLATFORM_H

#include "stm32f10x.h"

/* Defines to ensure platform dependance : Primer v1 or Primer v2 ------------*/

#ifdef _STM32F103RBT6_
#define PRIMER1 1
#define PRIMER_TYPE 1
#endif /*_STM32F103RBT6_*/
#ifdef _STM32F103VET6_
#define PRIMER2 1
#define PRIMER_TYPE 2
#endif /*_STM32F103VET6_*/

#if defined(PRIMER1) && defined(PRIMER2)
#error Both PRIMER1 and PRIMER2 are defined
#endif /*(PRIMER1 && PRIMER2)*/

#if !defined(PRIMER1) && !defined(PRIMER2)
#error Neither PRIMER1 nor PRIMER2 are defined
#endif /*(!PRIMER1 && !PRIMER2)*/

/*-------------------------------- Type definitions --------------------------*/
#include "circle_types.h"

typedef void ( *tAppPtrMgr )( coord_t sposX, coord_t sposY );

/*------------------------------ General -------------------------------------*/

/*------------------------------ Hardware configuration ----------------------*/
#if PRIMER1
#define JOYSTICK_AVAIL          0   // Joystic is managed or not
#define LED_INV                 0   // LED are active at level 0 or not
#define POWER_MNGT              0   // Battery charger present or not
#define SDCARD_AVAIL            0   // SDCard is managed or not
#define TOUCHSCREEN_AVAIL       0   // Touchscreeen is managed or not
#define AUDIO_AVAIL             0   // A codec is used, or only buzzer
#define MEMS_POINTER            1   // Menus are managed by Mems or not
#define BACKLIGHT_INTERFACE     1   // The backlight level is managed or not
#define MEMS_SPEC_MNGT          1   // The MEMS handler is called by special timer or not
#define MEMS_FULLINFO           1   // The MEMS handler computes all filters
#define SDCARD_SDIO             0   // SDCard is manage by SDIO peripheral or not
#define DISPLAY_TEMP            0   // Display internal CPU temperature
#define EXT_FONT                0   // Extended font management (several system fonts)
#endif

#if PRIMER2
#define JOYSTICK_AVAIL          1   // Joystic is managed or not
#define LED_INV                 0   // LED are active at level 0 or not
#define POWER_MNGT              1   // Battery charger present or not
#define SDCARD_AVAIL            1   // SDCard is managed or not
#define TOUCHSCREEN_AVAIL       1   // Touchscreeen is managed or not
#define AUDIO_AVAIL             1   // A codec is used, or only buzzer
#define MEMS_POINTER            1   // Menus are managed by Mems or not
#define BACKLIGHT_INTERFACE     1   // The backlight level is managed or not
#define MEMS_SPEC_MNGT          1   // The MEMS handler is called by special timer or not
#define MEMS_FULLINFO           1   // The MEMS handler computes all filters
#define SDCARD_SDIO             1   // SDCard is manage by SDIO peripheral or not
#define DISPLAY_TEMP            0   // Display internal CPU temperature
#define EXT_FONT                0   // Extended font management (several system fonts)
#endif

#define NODEBUG  __attribute__ ((section(".non_debuggable_code")))
#define NODEBUG2 __attribute__ ((section(".non_debuggable_code")))
#define IRQ __attribute__ ((interrupt ("IRQ")))
#define CONST_DATA const

#define CIRCLEOS_FAT_ADDRESS (0x08000104)
#define RAM_BASE             (0x20000000)
#define FA_TABLE             (0x08006000)

#if defined(PRIMER1)
// Data below moved to CircleStartup_spe_P1.s
//#define FLASH_SIZE           (128-16)       // 16 Ko flash for constants
//#define RAM_SIZE             (20-6)         // RAM available for applications
#endif
#if defined(PRIMER2)
// Data below moved to CircleStartup_spe_P2.s
//#define FLASH_SIZE           (512-120)      // 120 Ko flash for constants
//#define RAM_SIZE             (64-10)        // RAM available for applications
#endif

extern unsigned long _sdata;     /* start address for the .data section. defined in linker script */
#define CIRCLEOS_RAM_OFS  ( (unsigned long)&_sdata - RAM_BASE )
#define CIRCLEOS_RAM_BASE (RAM_BASE + CIRCLEOS_RAM_OFS)

#define APP_VOID ((tMenuItem*)(-1))

#define RCC_PERIPH_GPIO_CLOCK_CMD RCC_APB2PeriphClockCmd

/* Variables  ----------------------------------------------------------------*/
extern RCC_ClocksTypeDef RCC_ClockFreq;

/*-------------------------------   UTIL  ------------------------------------*/

#define ADC_NB_SAMPLES    10
#ifdef PRIMER2
#define ADC_NB_CHANNELS   5 /* = vbat + 3 touchscreen + temp*/
#else
#define ADC_NB_CHANNELS   2 /* = vbat + temp*/
#endif

#ifndef ADC_NB_CHANNELS
#define ADC_NB_CHANNELS   5
#endif /* ndef ADC_NB_CHANNELS*/

#define ADC_DMA_SIZE     ( ADC_NB_CHANNELS * ADC_NB_SAMPLES )

extern u16 ADC_ConvertedValue[ADC_DMA_SIZE];

extern CONST_DATA unsigned freqTIM2[6];

/*the following macro allows to adapt 'delay' defined by a number of tick to the real frequency of the tick.*/
/*we abusively adapt it using freqTIM2 because freqTIM2 and systick have always the same ratio*/
#define WEIGHTED_TIME(x)    ( ( (x) * freqTIM2[CurrentSpeed]) / freqTIM2[SPEED_MEDIUM] )
//TODO WEIGHTED_TIME is very heavy. It would be much simpler to change the max time when the CPU frequency is modified.

#define SPI_TIMEOUT                200

len_t       my_strlen( const u8* p );

/*-------------------------------   MEMS  ------------------------------------*/
#ifdef PRIMER1
#define RCC_APBxPERIPH_GPIOX_MEMS_CTRL      RCC_APB2Periph_GPIOB
#define RCC_APBxPERIPH_GPIOX_MEMS_CS        RCC_APB2Periph_GPIOD
#define SPIX_MEMS                           SPI2
#define RCC_APBxPERIPH_SPIX_MEMS            RCC_APB1Periph_SPI2
#define RCC_APBxPERIPH_MEMS_CLOCK_CMD       RCC_APB1PeriphClockCmd
#define GPIOX_MEMS_CTRL                     GPIOB
#define GPIOX_MEMS_CS                       GPIOD
#define GPIO_PIN_MEMS_CS                    GPIO_Pin_2
#define GPIO_PIN_MEMS_SCK                   GPIO_Pin_13
#define GPIO_PIN_MEMS_MISO                  GPIO_Pin_14
#define GPIO_PIN_MEMS_MOSI                  GPIO_Pin_15
#endif

#ifdef PRIMER2
#define RCC_APBxPERIPH_GPIOX_MEMS_CTRL      RCC_APB2Periph_GPIOA
#define RCC_APBxPERIPH_GPIOX_MEMS_CS        RCC_APB2Periph_GPIOE
#define SPIX_MEMS                           SPI1
#define RCC_APBxPERIPH_SPIX_MEMS            RCC_APB2Periph_SPI1
#define RCC_APBxPERIPH_MEMS_CLOCK_CMD       RCC_APB2PeriphClockCmd
#define GPIOX_MEMS_CTRL                     GPIOA
#define GPIOX_MEMS_CS                       GPIOE
#define GPIO_PIN_MEMS_CS                    GPIO_Pin_2
#define GPIO_PIN_MEMS_SCK                   GPIO_Pin_5
#define GPIO_PIN_MEMS_MISO                  GPIO_Pin_6
#define GPIO_PIN_MEMS_MOSI                  GPIO_Pin_7
#endif


/*----------------------------------   LED ------------------------------------*/
#ifdef PRIMER1
#define RCC_APBxPeriph_GPIOx_LED    RCC_APB2Periph_GPIOB
#define GPIOx_LED                   GPIOB
#define GPIO_Pin_LED0               GPIO_Pin_8
#define GPIO_Pin_LED1               GPIO_Pin_9
#endif

#ifdef PRIMER2
#define RCC_APBxPeriph_GPIOx_LED    RCC_APB2Periph_GPIOE
#define GPIOx_LED                   GPIOE
#define GPIO_Pin_LED0               GPIO_Pin_0
#define GPIO_Pin_LED1               GPIO_Pin_1
#endif

/*--------------------------------   ADC  ------------------------------------*/
#define ADC1_DR_Address    ((u32)0x4001244C)

/*--------------------------------   POWER  -------------------------------*/
#define GPIO_PWR_PERIPH         RCC_APB2Periph_GPIOC
#define GPIOx_PWR               GPIOC
#define GPIOx_PWR_STAT          GPIOC
#define GPIO_PWR_LOADING_PIN    GPIO_Pin_6  //Status1
#define GPIO_PWR_DONE_PIN       GPIO_Pin_7  //Status2
#define BAT_FULL_VOLTAGE        4200
#define BAT_LOW_VOLTAGE         3750
#define BAT_EMPTY_VOLTAGE       3500
#ifdef PRIMER1
#define NO_BAT_VOLTAGE          3200 //2850
#else
#define NO_BAT_VOLTAGE          3000
#endif

void POWER_Reset_Time( void );
void POWER_Set_Time( void );

/*--------------------------------   BUTTON  ---------------------------------*/
/*BUTTON IS ON PA0 (or PA8 for PRIMER2)*/
#ifdef PRIMER1
#define  GPIO_BUTTON_PIN         GPIO_Pin_0
#endif
#ifdef PRIMER2
#define  GPIO_BUTTON_PIN         GPIO_Pin_8
#endif
#define  GPIOx_BUTTON            GPIOA
#define  GPIO_BUTTON_PERIPH      RCC_APB2Periph_GPIOA

/*--------------------------------  JOYSTICK  --------------------------------*/
#ifdef PRIMER2
#define  GPIO_JOYSTICK_L_PIN     GPIO_Pin_3
#define  GPIO_JOYSTICK_R_PIN     GPIO_Pin_4
#define  GPIO_JOYSTICK_U_PIN     GPIO_Pin_5
#define  GPIO_JOYSTICK_D_PIN     GPIO_Pin_6
#define  GPIOx_JOYSTICK          GPIOE
#define  GPIO_JOYSTICK_PERIPH    RCC_APB2Periph_GPIOE
#endif

/*--------------------------------   POINTER  --------------------------------*/

/*----------------------------------   LCD   ---------------------------------*/
#define LCD_HIGH_DEF 0            // 1 => magnitude 2 for system strings

#include "lcd_spe.h"
#include "font_spe.h"

#ifdef PRIMER2
void LCD_FSMC_Init( void );
#endif

/*----------------------------------   DRAW   --------------------------------*/

/*--------------------------------   BUZZER  ---------------------------------*/
#define SYSTEM_CLOCK RCC_ClockFreq.SYSCLK_Frequency

/*--------------------------------   AUDIO CODEC  ----------------------------*/
/* Exported defines ----------------------------------------------------------*/
#define CONTROL_REGISTER_0 0
#define SPI2_I2SPR (SPI2_BASE + 0x20)
#define SPI3_I2SPR (SPI3_BASE + 0x20)
#define AUDIO_MIN_ATTENUATION 4        /* Min attenuation in dB*/

/*---------------------------------   MENU   -----------------------------------*/
/* Exported defines ----------------------------------------------------------*/
#define LCD_HIGH_DEF 0          // 0 => magnitude 1 for system strings

#define MENU_MAXITEM 7
#define MAX_APP_MENU_SIZE 10
#define MAXAPP  64
#define MAX_MENUAPP_SIZE 3

#define MAXBTIME     4           /*!< The stability in the menu.           */
#define MAXLEVEL     5

#define LEFT_MARGIN  10
#define WIDTH_SQUARE 20
#define BAR_BOTTOM   50
#define BAR_LEFT     10

#define TIME_MARGIN  40
#define TIME_BOTTOM  50
#define TIME_TITLE_X 8
#define TIME_TITLE_Y 100
#define TIME_TITLE_H 38


/*--------------------------------   BACKLIGHT  ------------------------------*/

/*--------------------------------   TOUCHSCR  -------------------------------*/

/*--------------------------------   TOOLBAR  --------------------------------*/
#define TOOLBAR_MAXITEMS 4
#define TB_NB_BUTTONS    4
#define BUTTON_HEIGHT    32
#define BUTTON_WIDTH     32
#define ICON_HEIGHT      32
#define ICON_WIDTH       32

/*--------------------------------   POWER  -----------------------------------*/

/*--------------------------------   LIST  -----------------------------------*/
/* Exported defines ----------------------------------------------------------*/
#define LIST_MAXITEM 64
#define DELTA_Y         2//(CHAR_HEIGHT / 7)   /* Number of pixels moved during 1 move*/
#ifdef PRIMER1
#define LCD_DMA_SIZE (48*3)
#else
#define LCD_DMA_SIZE (128 * DELTA_Y * 3)    /* 128 pixels width * DELTA_Y pixels height * (3 bytes / pixel)*/
#endif

/*--------------------------------   RTC  --------------------------------------*/
/* Exported defines ----------------------------------------------------------*/
/* Backup registers*/
#define BKP_SYS1     1          /* current application*/
#define BKP_SYS2     2          /* speed, joystick...*/
#define BKP_SYS3     3          /* backlight value*/
#define BKP_SYS4     4
#define BKP_SYS5     5
#define BKP_SYS6     6
#define BKP_USER1    7
#define BKP_USER2    8
#define BKP_USER3    9
#define BKP_USER4    10
#define BKP_SYS7     11         /* Touchscreen option */
#define BKP_SYS8     12         /* Touchscreen calibration points */
#define BKP_SYS9     13
#define BKP_SYS10    14
#define BKP_SYS11    15
#define BKP_SYS12    16
#define BKP_SYS13    17

#define BKPMASK_S2_PLL      0x0007
#define BKPMASK_S2_SPEAKER  0x0008
#define BKPMASK_S2_JOYSTICK 0x0010
#define BKPMASK_S2_MEMS     0x0020
#define BKPMASK_S2_MUTE     0x0040
#define BKPMASK_S2_AUTORUN  0x0080
#define BKPMASK_S2_VOLUME   0x0F00
#define BKPMASK_S2_BUZZER   0x1000
#define BKPMASK_S7_TCHSCR   0x0200

#define BKP_BKLIGHT  (BKP_SYS3)

#define BKP_TS_X0    (BKP_SYS8)
#define BKP_TS_Y0    (BKP_SYS9)
#define BKP_TS_X1    (BKP_SYS10)
#define BKP_TS_Y1    (BKP_SYS11)
#define BKP_TS_X2    (BKP_SYS12)
#define BKP_TS_Y2    (BKP_SYS13)

/*--------------------------------- USB ------------------------------*/
#define USB_DISCONNECT_PORT               GPIOD
#define USB_DISCONNECT_PIN                GPIO_Pin_3
#define RCC_APBxPeriph_GPIO_DISCONNECT    RCC_APB2Periph_GPIOD

/*--------------------------------- SDIO -----------------------------*/
#define SDIO_INIT_CLK_DIV                  ((uint8_t)0xB2)
#define SDIO_TRANSFER_CLK_DIV              ((uint8_t)0x1) 
#define SDIO_FIFO_ADDRESS                ((uint32_t)0x40018080)

/*--------------------------------- Application ------------------------------*/
#ifndef _MENU_CODE_DEFINED
#define _MENU_CODE_DEFINED
enum  MENU_code
{
    MENU_LEAVE              = 0,
    MENU_CONTINUE           = 1,
    MENU_REFRESH            = 2,
    MENU_CHANGE             = 3,
    MENU_CONTINUE_COMMAND   = 4,
    MENU_LEAVE_AS_IT        = 5,
    MENU_RESTORE_COMMAND    = 6
};

typedef struct
{
    const u8* Text;
    enum MENU_code( *Fct_Init )( void );
    enum MENU_code( *Fct_Manage )( void );
    enumset_t fMenuFlag;
} tMenuItem;

extern tMenuItem*(( *ApplicationTable )[] );

#endif //_MENU_CODE_DEFINED

#endif /*__CIRCLE_H */
