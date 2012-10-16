/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     Util_spe2.c
* @brief    Various harware specific utilities for CircleOS.
* @author   YRT
* @date     05/2011
* @note     Platform Open4 STM32L
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private defines -----------------------------------------------------------*/
#define  VDD_VOLTAGE_MV  2830 /* Voltage (mV) of the STM32*/
#define BACKUP_CONFIGURED_MARKER 0xC0CA

/* Private typedef -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
RCC_ClocksTypeDef    RCC_ClockFreq;
s16                  VBat = -1;
s16                  Vdd  = VDD_VOLTAGE_MV;


// Backup registers are blanked upon reflash
__attribute__ ((section(".data_eeprom")))
backup_t BackupRegs[20]   = {0};
__attribute__ ((section(".data_eeprom")))
backup_t Backupconfigured = 0;

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
    return ( Backupconfigured == BACKUP_CONFIGURED_MARKER );
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
*   Uses pin 12 (CX_ADC2) of the extension connector
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

/*******************************************************************************
*
*                    UTIL_GetVdd
*
*******************************************************************************/
/**
* Retrieves the Vdd voltage. The STM32L15x is able to retrieve VREFINT
* from its ADC. VREFINT is 1.225 V, so we just have to invert the value
* read to get an accurate Vdd measurement.
* @return current voltage in thousandths of Volts. For example,
* 3230 would be 3.230 V
*/
/******************************************************************************/
u16 UTIL_GetVdd( void )
{
    u16 value;
    vs32 i;

    // Calculate the mean value
    value = 0;
    for ( i = 0; i < ADC_NB_SAMPLES; i++ )
    {
        value += ADC_ConvertedValue[5 + i*ADC_NB_CHANNELS];
    }
    value /= ADC_NB_SAMPLES;

    value &= 0xFFF;

    return 4096UL * 1225 / value;
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
    vbat = ( vbat * UTIL_GetVdd() ) / 0x1000;

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
    u16 value;
    vs32 i;

    // Calculate the mean value
    value = 0;
    for ( i = 0; i < ADC_NB_SAMPLES; i++ )
    {
        value += ADC_ConvertedValue[1 + i*ADC_NB_CHANNELS];
    }
    value /= ADC_NB_SAMPLES;

    value &= 0xFFF;

    //------------------------------------------------------------------------------
    // The transfer function for the temperature sensor is:
    // Vout = 1.8663 - 0.01169 * T  (values in Volts and °C)
    // The transfer function for the ADC is:
    //      Vread = Vout x 0xFFF / Vdd
    //      Vread = Vout x 4095000 / VddmV  (VddmV is Vdd in hundredths of Volts)
    // So the temperature Th (in hundredths of °C) is defined as:
    //      Th = (1.8663 - Vout) / 0.0001169
    //      Th = (1.8663 - Vout) * 8554.32
    //         = (1.8663 - Vread * VddmV / 4095000) * 8554.32
    //         = 15964.92 - 0.002089 * Vread * VddmV
    //         = 15964.92 - 0.002089 * Vread * VddmV
    // For instance, if Vdd = 3.15V, T = 20°C, Vout = 1.633V, Vread = 2123, VddmV = 3150
    //      Th = 15964.92 - 0.002089 * 2123 * 3150 = 2000 (= 20°C)
    //------------------------------------------------------------------------------

    temp =  ( 15964.92 - ( 0.002089 * value * UTIL_GetVdd() ) );

    // Fahrenheit = 32 + 9 / 5 * Celsius
    if ( fTemperatureInFahrenheit )
    {
        temp = 32000 + ( 9 * temp ) / 5 ;
    }

    return temp / 10;
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

    if (( speed < SPEED_VERY_LOW ) || ( speed > SPEED_VERY_HIGH ) )
    {
        speed = SPEED_MEDIUM;
    }

    CurrentSpeed = speed;

    switch ( speed )
    {
        // 9 MHz
    case SPEED_VERY_LOW  :
        /* PLL configuration: PLLCLK = HSE * 1.5 = 9 MHz */
        RCC->CFGR &= ( uint32_t )~( RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL | RCC_CFGR_PLLDIV);
        RCC->CFGR |= ( uint32_t )( RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PLLMUL3 | RCC_CFGR_PLLDIV4 );
        break;

        // 12MHz
    case SPEED_LOW       :
        /* PLL configuration: PLLCLK = HSE * 1 = 12 MHz */
        RCC->CFGR &= ( uint32_t )~( RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL | RCC_CFGR_PLLDIV);
        RCC->CFGR |= ( uint32_t )( RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PLLMUL4 | RCC_CFGR_PLLDIV4 );
        break;

        // 18Mhz
    default              :
        CurrentSpeed = SPEED_MEDIUM;
    case SPEED_MEDIUM    :
        /* PLL configuration: PLLCLK = HSE * 1.5 = 18 MHz */
        RCC->CFGR &= ( uint32_t )~( RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL | RCC_CFGR_PLLDIV);
        RCC->CFGR |= ( uint32_t )( RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PLLMUL6 | RCC_CFGR_PLLDIV4 );
        break;

        // 24MHz
    case SPEED_HIGH      :
        /* PLL configuration: PLLCLK = HSE * 2 = 24 MHz */
        RCC->CFGR &= ( uint32_t )~( RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL | RCC_CFGR_PLLDIV);
        RCC->CFGR |= ( uint32_t )( RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PLLMUL6 | RCC_CFGR_PLLDIV3 );
        break;

        // 32MHz
    case SPEED_VERY_HIGH :
        /* PLL configuration: PLLCLK = HSE * 8 / 3 = 32 MHz */
        RCC->CFGR &= ( uint32_t )~( RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL | RCC_CFGR_PLLDIV);
        RCC->CFGR |= ( uint32_t )( RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PLLMUL8 | RCC_CFGR_PLLDIV3 );
        break;
    }

    /* Enable PLL */
    RCC_PLLCmd( ENABLE );

    /* Wait till PLL is ready */
    while (( RCC->CR & RCC_CR_PLLRDY ) == 0 )
    {
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig( RCC_SYSCLKSource_PLLCLK );

    /* Wait till PLL is used as system clock source */
    while (( RCC->CFGR & ( uint32_t )RCC_CFGR_SWS ) != ( uint32_t )0x0C )
    {
    }

    /* This function fills a RCC_ClocksTypeDef structure with the current frequencies
    of different on chip clocks (for debug purpose) */
    RCC_GetClocksFreq( &RCC_ClockFreq );

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
    // Check that the register index is not too high
    if ( BKP_DR < COUNTOF( BackupRegs ) )
        return BackupRegs[BKP_DR];
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
*  @param[in]  BKP_DR   Specifies the Data Backup Register.
*                       This parameter can be BKP_DRx where x:[1, 20]
*  @param[in]  Data     The data to write.
*
*  @note : the STM32L does not own backup register, so we save into data EEPROM 
*
**/
/********************************************************************************/
void UTIL_WriteBackupRegister( index_t BKP_DR, backup_t Data )
{
    // Check that the register index is not too high
    if ( BKP_DR < COUNTOF( BackupRegs ) )
    {
        DATA_EEPROM_Unlock();
    
        // Clear All pending flags
        FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR
                  | FLASH_FLAG_SIZERR | FLASH_FLAG_OPTVERR); 
    
        // Save the data into register
        FLASH_Status cr = DATA_EEPROM_ProgramHalfWord( (uint32_t) &BackupRegs[BKP_DR], Data);
    
        // Mark backup register marked as configured
        if (cr == FLASH_COMPLETE)
            cr = DATA_EEPROM_ProgramHalfWord( (uint32_t) &Backupconfigured, BACKUP_CONFIGURED_MARKER);
    
        DATA_EEPROM_Lock();
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


