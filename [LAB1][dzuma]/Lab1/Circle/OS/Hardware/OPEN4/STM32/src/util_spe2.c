/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     Util_spe2.c
* @brief    Various harware specific utilities for CircleOS.
* @author   YRT
* @date     09/2009
* @note     Platform Open4 STM32C & STM32E
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private defines -----------------------------------------------------------*/
#define  VDD_VOLTAGE_MV  3150 /* Voltage (mV) of the STM32*/

/* Private typedef -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
RCC_ClocksTypeDef    RCC_ClockFreq;
s16                  VBat = -1;

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/* External variables ---------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/

/*******************************************************************************
*
*                                UTIL_isBackupRegisterConfigured
*
*******************************************************************************/
/**
*
*  Checks whether backup registers are configured or not.
*
*  @return The configuration status of backup registers.
*
**/
/******************************************************************************/
bool UTIL_isBackupRegisterConfigured( void )
{
    int i;

    for ( i = 1 ; i <= 11 ; i++ )
    {
        if ( UTIL_ReadBackupRegister( i ) != 0 )
            return TRUE;
    }

    return FALSE;
}


/*******************************************************************************
*
*                                SetBitTimingAnalysis
*
*******************************************************************************/
/**
*
*   To analyse systick timing with a scope
*
*   Uses pin 18 (CX_USART_Rx) of the extension connector
*
*   @param[in]  set   set or reset the output.
*
**/
/********************************************************************************/
void SetBitTimingAnalysis( u8 set )
{
    if ( set )
    {
        GPIO_WriteBit( GPIOA, GPIO_Pin_3, Bit_SET );
    }
    else
    {
        GPIO_WriteBit( GPIOA, GPIO_Pin_3, Bit_RESET );
    }
}

/// @endcond

/*******************************************************************************
*
*                    UTIL_GetBat
*
*******************************************************************************/
/**
*
*  Return the battery tension in mV.
*
*  @return Battery tension in mV.
*
**/
/******************************************************************************/
u16 UTIL_GetBat( void )
{
    u16 vbat;
    vs32 i;

    // Measures VBAT and calculates the mean value
    vbat = 0;
    for ( i = 0; i < ADC_NB_SAMPLES; i++ )
    {
        vbat += ADC_ConvertedValue[0 + i * ADC_NB_CHANNELS];
    }
    vbat = vbat / ADC_NB_SAMPLES;

    vbat = vbat & 0xFFF;
    vbat = ( vbat * VDD_VOLTAGE_MV ) / 0x1000;

    vbat *= 2; //Divider bridge  Vbat <-> 1M -<--|-->- 1M <-> Gnd,

    if ( VBat == -1 )
    {
        VBat = vbat;
    }
    else
    {
        VBat = ( VBat >> 1 ) + ( vbat >> 1 );
    }
    return VBat;
}

/*******************************************************************************
*
*                    UTIL_GetTemp
*
*******************************************************************************/
/**
*
*  Return the Temperature: degrees / 10, Celcius or Fahrenheit.
*
*  @return The temperature (C or F) (averaging of several channels).
*
**/
/******************************************************************************/
u16 UTIL_GetTemp( void )
{
    u16 temp;
    vs32 i;

    // Calculate the mean value
    temp = 0;
    for ( i = 0; i < ADC_NB_SAMPLES; i++ )
    {
        temp += ADC_ConvertedValue[1 + i*ADC_NB_CHANNELS];
    }
    temp = temp / ADC_NB_SAMPLES;

    temp = temp & 0xFFF;
    temp = ( temp * VDD_VOLTAGE_MV ) / 0x1000;  //finds mV
    temp = ((( 1430 - temp ) * 100000 ) / 4300 ) + 25000; //gives approx temp x 1000 degrees C

    //Fahrenheit = 32 + 9 / 5 * Celsius
    if ( fTemperatureInFahrenheit )
    {
        temp = 32000 + ( 9 * temp ) / 5 ;
    }

    return temp / 100;
}


/*******************************************************************************
*
*                                UTIL_SetPll
*
*******************************************************************************/
/**
*
*  Set clock frequency (lower to save energy)
*
*  @param [in]   speed  New clock speed from very low to very fast.
*  @note :  STM32F10X_CL = STM32F107
*           STM32L1XX_MD = STM32L152 
*           others       = STM32E
*
**/
/******************************************************************************/
void UTIL_SetPll( enum eSpeed speed )
{
    /* Select internal clok as system clock source */
    RCC_SYSCLKConfig( RCC_SYSCLKSource_HSI );

    /* Disable PLL */
    RCC_PLLCmd( DISABLE );
#ifdef STM32F10X_CL
    RCC_PLL2Cmd( DISABLE );
#endif

    if (( speed < SPEED_VERY_LOW ) || ( speed > SPEED_VERY_HIGH ) )
    {
        speed = SPEED_MEDIUM;
    }

    CurrentSpeed = speed;

    switch ( speed )
    {
        // 18 MHz
    case SPEED_VERY_LOW  :
#ifdef STM32F10X_CL
        // Source PLLCLK = PLL2
        RCC->CFGR2 &= ( uint32_t )~( RCC_CFGR2_PREDIV2 | RCC_CFGR2_PLL2MUL  |
                                     RCC_CFGR2_PREDIV1 | RCC_CFGR2_PREDIV1SRC );
        RCC->CFGR2 |= ( uint32_t )( RCC_CFGR2_PREDIV2_DIV4 | RCC_CFGR2_PLL2MUL12 |
                                    RCC_CFGR2_PREDIV1SRC_PLL2 | RCC_CFGR2_PREDIV1_DIV16 | RCC_CFGR2_I2S2SRC );
        /* PLL configuration: PLLCLK = (PLL2CLK / 16) * 6.5 = 17.97 MHz */
        RCC->CFGR &= ( uint32_t )~( RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL );
        RCC->CFGR |= ( uint32_t )( RCC_CFGR_PLLSRC_PREDIV1 | RCC_CFGR_PLLMULL6_5 );
#else
        /* PLLCLK = 6MHz * 3 = 18 MHz */
        RCC_PLLConfig( RCC_PLLSource_HSE_Div2, RCC_PLLMul_3 );
#endif
        break;

        // 24MHz
    case SPEED_LOW       :
#ifdef STM32F10X_CL
        // Source PLLCLK = PLL2
        RCC->CFGR2 &= ( uint32_t )~( RCC_CFGR2_PREDIV2 | RCC_CFGR2_PLL2MUL |
                                     RCC_CFGR2_PREDIV1 | RCC_CFGR2_PREDIV1SRC );
        RCC->CFGR2 |= ( uint32_t )( RCC_CFGR2_PREDIV2_DIV4 | RCC_CFGR2_PLL2MUL12 |
                                    RCC_CFGR2_PREDIV1SRC_PLL2 | RCC_CFGR2_PREDIV1_DIV9 | RCC_CFGR2_I2S2SRC );
        /* PLL configuration: PLLCLK = (PLL2CLK / 9) * 5 = 24.58 MHz */
        RCC->CFGR &= ( uint32_t )~( RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL );
        RCC->CFGR |= ( uint32_t )( RCC_CFGR_PLLSRC_PREDIV1 | RCC_CFGR_PLLMULL5 );
#else
        /* PLLCLK = 12MHz * 2 = 24 MHz */
        RCC_PLLConfig( RCC_PLLSource_HSE_Div1, RCC_PLLMul_2 );
#endif
        break;

        // 36MHz
    default              :
        CurrentSpeed = SPEED_MEDIUM;
    case SPEED_MEDIUM    :
#ifdef STM32F10X_CL
        // Source PLLCLK = PLL2
        RCC->CFGR2 &= ( uint32_t )~( RCC_CFGR2_PREDIV2 | RCC_CFGR2_PLL2MUL |
                                     RCC_CFGR2_PREDIV1 | RCC_CFGR2_PREDIV1SRC );
        RCC->CFGR2 |= ( uint32_t )( RCC_CFGR2_PREDIV2_DIV4 | RCC_CFGR2_PLL2MUL12 |
                                    RCC_CFGR2_PREDIV1SRC_PLL2 | RCC_CFGR2_PREDIV1_DIV8 | RCC_CFGR2_I2S2SRC );
        /* PLL configuration: PLLCLK = (PLL2CLK / 8) * 6.5 = 35.94 MHz */
        RCC->CFGR &= ( uint32_t )~( RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL );
        RCC->CFGR |= ( uint32_t )( RCC_CFGR_PLLSRC_PREDIV1 | RCC_CFGR_PLLMULL6_5 );
#else
        /* PLLCLK = 12MHz * 3 = 36 MHz */
        RCC_PLLConfig( RCC_PLLSource_HSE_Div1, RCC_PLLMul_3 );
#endif
        break;

        // 48MHz
    case SPEED_HIGH      :
#ifdef STM32F10X_CL
        // Source PLLCLK = PLL2
        RCC->CFGR2 &= ( uint32_t )~( RCC_CFGR2_PREDIV2 | RCC_CFGR2_PLL2MUL |
                                     RCC_CFGR2_PREDIV1 | RCC_CFGR2_PREDIV1SRC );
        RCC->CFGR2 |= ( uint32_t )( RCC_CFGR2_PREDIV2_DIV4 | RCC_CFGR2_PLL2MUL12 |
                                    RCC_CFGR2_PREDIV1SRC_PLL2 | RCC_CFGR2_PREDIV1_DIV8 | RCC_CFGR2_I2S2SRC );
        /* PLL configuration: PLLCLK = (PLL2CLK / 8) * 9 = 49.77 MHz */
        RCC->CFGR &= ( uint32_t )~( RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL );
        RCC->CFGR |= ( uint32_t )( RCC_CFGR_PLLSRC_PREDIV1 | RCC_CFGR_PLLMULL9 );
#else
        /* PLLCLK = 12MHz * 4 = 48 MHz */
        RCC_PLLConfig( RCC_PLLSource_HSE_Div1, RCC_PLLMul_4 );
#endif
        break;

        // 72MHz
    case SPEED_VERY_HIGH :
#ifdef STM32F10X_CL
        // Source PLLCLK = PLL2
        RCC->CFGR2 &= ( uint32_t )~( RCC_CFGR2_PREDIV2 | RCC_CFGR2_PLL2MUL |
                                     RCC_CFGR2_PREDIV1 | RCC_CFGR2_PREDIV1SRC );
        RCC->CFGR2 |= ( uint32_t )( RCC_CFGR2_PREDIV2_DIV4 | RCC_CFGR2_PLL2MUL12 |
                                    RCC_CFGR2_PREDIV1SRC_PLL2 | RCC_CFGR2_PREDIV1_DIV4 | RCC_CFGR2_I2S2SRC );
        /* PLL configuration: PLLCLK = (PLL2CLK / 4) * 6.5 = 71.88 MHz */
        RCC->CFGR &= ( uint32_t )~( RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL );
        RCC->CFGR |= ( uint32_t )( RCC_CFGR_PLLSRC_PREDIV1 | RCC_CFGR_PLLMULL6_5 );
#else
        /* PLLCLK = 12MHz * 6 = 72 MHz */
        RCC_PLLConfig( RCC_PLLSource_HSE_Div1, RCC_PLLMul_6 );
#endif
        break;
    }

#ifdef STM32F10X_CL
    /* Enable PLL2 */
    RCC_PLL2Cmd( ENABLE );

    /* Wait till PLL2 is ready */
    while (( RCC->CR & RCC_CR_PLL2RDY ) == 0 )
    {
    }
#endif

    /* Enable PLL */
    RCC_PLLCmd( ENABLE );

    /* Wait till PLL is ready */
    while (( RCC->CR & RCC_CR_PLLRDY ) == 0 )
    {
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig( RCC_SYSCLKSource_PLLCLK );

    /* Wait till PLL is used as system clock source */
    while (( RCC->CFGR & ( uint32_t )RCC_CFGR_SWS ) != ( uint32_t )0x08 )
    {
    }

    /* This function fills a RCC_ClocksTypeDef structure with the current frequencies
    of different on chip clocks (for debug purpose) */
    RCC_GetClocksFreq( &RCC_ClockFreq );

#if AUDIO_AVAIL
#ifndef STM32F10X_CL
    extern const u16 I2S_PrescalerVal[];
    // Set the prescaler values for the I2S (audio codec) according to the PLL
    *( u16* )SPI2_I2SPR = I2S_PrescalerVal[speed-1];
#endif
#endif
}


/*******************************************************************************
*
*                                UTIL_ReadBackupRegister
*
*******************************************************************************/
/**
*
*  Reads data from the specified Data Backup Register.
*
*  @param[in]  BKP_DR   Specifies the Data Backup Register. This parameter can be BKP_DRx where x:[1, 10]
*
*  @return  The content of the specified Data Backup Register.
*
**/
/******************************************************************************/
backup_t UTIL_ReadBackupRegister( index_t BKP_DR )
{
    if ( BKP_DR < 11 )
    {
        return ( *( vu16* )( BKP_BASE + 4 * BKP_DR ) );
    }
    else if ( BKP_DR < 43 )
    {
        return ( *( vu16* )( BKP_BASE + 4 * ( BKP_DR + 5 ) ) );
    }
    else return 0;
}

/*******************************************************************************
*
*                                UTIL_WriteBackupRegister
*
*******************************************************************************/
/**
*
*  Writes data to the specified Data Backup Register.
*
*  @param[in]  BKP_DR   Specifies the Data Backup Register. This parameter can be BKP_DRx where x:[1, 10]
*  @param[in]  Data     The data to write.
*
**/
/********************************************************************************/
void UTIL_WriteBackupRegister( index_t BKP_DR, backup_t Data )
{
    if ( BKP_DR < 11 )
    {
        *( vu16* )( BKP_BASE + 4 * BKP_DR ) = Data;
    }
    else if ( BKP_DR < 43 )
    {
        *( vu16* )( BKP_BASE + 4 *( BKP_DR + 5 ) ) = Data;
    }
}

/*******************************************************************************
*
*                                UTIL_GetPrimerType
*
*******************************************************************************/
/**
*
*  Get the current type of the PRIMER.
*
*  @return  1 = PRIMER 1,
*           2 = PRIMER 2,
*           4 = OPEN4.
*
**/
/********************************************************************************/
u16 UTIL_GetPrimerType( void )
{
    return PRIMER_TYPE;
}
