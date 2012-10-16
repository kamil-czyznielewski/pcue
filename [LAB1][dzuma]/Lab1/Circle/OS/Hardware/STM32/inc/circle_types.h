/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     Circle_types.h
* @brief    STM32 specific types declaration.
* @author   YRT
* @date     05/2010
* @version  4.0
*
*
**/
/******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CIRTYPE_H
#define __CIRTYPE_H


// For old ST librairies compatibility
#if !defined (__STM32F10x_H) && !defined(__STM32F2xx_H)

typedef int32_t s32;
typedef int16_t s16;
typedef int8_t  s8;

typedef const int32_t sc32;  /*!< Read Only */
typedef const int16_t sc16;  /*!< Read Only */
typedef const int8_t  sc8;   /*!< Read Only */

typedef volatile int32_t  vs32;
typedef volatile int16_t  vs16;
typedef volatile int8_t   vs8;

typedef volatile const int32_t vsc32;  /*!< Read Only */
typedef volatile const int16_t vsc16;  /*!< Read Only */
typedef volatile const int8_t  vsc8;   /*!< Read Only */

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef const uint32_t uc32;  /*!< Read Only */
typedef const uint16_t uc16;  /*!< Read Only */
typedef const uint8_t  uc8;   /*!< Read Only */

typedef volatile uint32_t vu32;
typedef volatile uint16_t vu16;
typedef volatile uint8_t  vu8;

typedef volatile const uint32_t vuc32;  /*!< Read Only */
typedef volatile const uint16_t vuc16;  /*!< Read Only */
typedef volatile const uint8_t  vuc8;   /*!< Read Only */

#endif  // !defined (__STM32F10x_H) && !defined(__STM32F2xx_H)

#ifndef __cplusplus
typedef enum {FALSE = 0, TRUE = !FALSE} bool;
#endif

#if !defined (__STM32F10x_H)

#if !defined(__STM32F2xx_H) && !defined(__STM32L1XX_H)
typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITStatus;

typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;
#define IS_FUNCTIONAL_STATE(STATE) (((STATE) == DISABLE) || ((STATE) == ENABLE))

typedef enum {ERROR = 0, SUCCESS = !ERROR} ErrorStatus;

#endif  // #if !defined(__STM32F2xx_H) && !defined(__STM32L1XX_H)
#endif  // #if !defined (__STM32F10x_H)

/* Type definitions --------------------------*/
typedef s32 int_t;          // Generic int
typedef u32 uint_t;         // Generic unsigned
typedef s32 ret_t;          // Define the CircleOS functions return type
typedef s16 coord_t;        // Screen coordinates type
typedef s32 coord_n_t;      // Menu coordinates type
typedef u16 color_t;        // Pixel color type
typedef u32 delay_t;        // Used to count a number a milliseconds
typedef s32 mag_t;          // Magnifying coefficient
typedef u32 lcdt_t;         // LCD data type
typedef u32 backlight_t;    // Backlight frequency
typedef u32 counter_t;      // Seconds counter
typedef s32 index_t;        // Indexes (applications for instance)
typedef s32 len_t;          // Lengths (strings etc)
typedef s32 enumset_t;      // Contains a set of enums (or'ed values)
typedef u16 backup_t;       // Backup registers
typedef s32 divider_t;      // Divider

#endif /*__CIRTYPE_H */
