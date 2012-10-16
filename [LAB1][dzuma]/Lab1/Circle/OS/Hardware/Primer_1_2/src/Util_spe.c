/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     Util_spe.c
* @brief    Various harware specific  utilities for CircleOS.
* @author   YRT
* @date     09/2009
* @Note     Split from Util.c
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/

#include "circle.h"
#include "adc_spe.h"

/// @cond Internal

/* Private defines -----------------------------------------------------------*/
#ifdef PRIMER1
#define  GPIO_USB_PIN   GPIO_Pin_1
#define  GPIOx_USB      GPIOA
#define  VDD_VOLTAGE_MV  3300 /* Voltage (mV) of the STM32*/
#endif

#ifdef PRIMER2
#define  GPIO_USB_PIN   GPIO_Pin_3
#define  GPIOx_USB      GPIOD
#define  VDD_VOLTAGE_MV  2880 /* Voltage (mV) of the STM32*/
#endif

/* Private typedef -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
RCC_ClocksTypeDef    RCC_ClockFreq;
s16                  VBat = -1;        // YRT 20081024 the variable is signed
bool                 ClockBug = 0;     // Detection of oscillator bug on Primer2

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
void fct_dummy() {;}

/* External variables ---------------------------------------------------------*/
extern vu32 TimingDelay;

/* Public functions ----------------------------------------------------------*/

/*******************************************************************************
*
*                    Delayms
*
*******************************************************************************/
/**
*
*  Delay of about x ms ?
*
*  @note Not in main.c to avoid inlining.
*
**/
/******************************************************************************/
void Delayms( u32 nTime )
{
    TimingDelay = nTime;

    while ( TimingDelay != 0 );
}

/*******************************************************************************
*
*                                starting_delay
*
*******************************************************************************/
/**
*  Delay at the startup, depending on the platform
*
*
**/
/*******************************************************************************/
void starting_delay( void )
{
    vs32 i, j;
    for ( i = 0; i < 100; i++ )
        for ( j = 0; j < 1300; j++ )
            fct_dummy();
}

/*******************************************************************************
/**
*   SetBitTimingAnalysis
*
*   To analyse systick timing with a scope
*   Uses pin 8 (WS) of the extension connector (PRIMER 1)
*   Uses pin 18 (CX_USART_Rx) of the extension connector (PRIMER 2)
*
*   @param[in]  set   set or reset the output.
*
**/
/********************************************************************************/
NODEBUG void SetBitTimingAnalysis( u8 set )
{
    if ( set )
    {
#ifdef PRIMER1
        GPIO_WriteBit( GPIOA, GPIO_Pin_6, Bit_SET );
#endif
#ifdef PRIMER2
        GPIO_WriteBit( GPIOA, GPIO_Pin_3, Bit_SET );
#endif
    }
    else
    {
#ifdef PRIMER1
        GPIO_WriteBit( GPIOA, GPIO_Pin_6, Bit_RESET );
#endif
#ifdef PRIMER2
        GPIO_WriteBit( GPIOA, GPIO_Pin_3, Bit_RESET );
#endif
    }
}

/******************************************************************************
*
*                                Sleep_Call
*
******************************************************************************/
/* To respect Misra rule 3 :  */
/*"Assembly language functions that are called from C should be written as C functions containing only in-line assembly language.  */
/*In-line assembly language should not be embedded in normal C code."  */

void Sleep_Call( void )
{
    asm( "wfi" );
}

/******************************************************************************
*
*                                Check_CPU_Clock
*
*   This routine is to fix a hardware bug on some Primer2
*   (oscillator = 36 MHz instead 12 MHz)
*
******************************************************************************/
#ifdef PRIMER2
NODEBUG void Check_CPU_Clock( void )
{
    // Set CPU clock to 24 MHz => systick = 1 ms
    enum eSpeed CPUClockBck = UTIL_GetPll();
    UTIL_SetPll( SPEED_LOW );

    // Wait for start reference pulse
    RTC_ClearFlag( RTC_FLAG_SEC );
    while ( RTC_GetFlagStatus( RTC_FLAG_SEC ) == RESET )
        {;}

    // Wait for end reference pulse, 1s later
    TimingDelay = 10000;

    RTC_ClearFlag( RTC_FLAG_SEC );
    while ( RTC_GetFlagStatus( RTC_FLAG_SEC ) == RESET )
        {;}

    // We check the number of systick pulses (= 9000 at 24 MHz)
    ClockBug = ( TimingDelay < 8500 );

    // Restore CPU clock
    UTIL_SetPll( CPUClockBck );
}
#endif

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

    // Measure VBAT
    //    vbat = ADC_ConvertedValue[0];  //*( (u16*)ADC1_DR_Address );      // <=== note changed

    // Calculate the mean value       // YRT 20081023
    vbat = 0;
    for ( i = 0; i < ADC_NB_SAMPLES; i++ )
    {
        vbat += ADC_ConvertedValue[0 + i*ADC_NB_CHANNELS];
    }
    vbat = vbat / ADC_NB_SAMPLES;

    vbat = vbat & 0xFFF;
    vbat = ( vbat * VDD_VOLTAGE_MV ) / 0x1000;

// #ifdef PRIMER2 YRT20100126 The divider is also present on Primer1 !
    vbat *= 2; //Divider bridge  Vbat <-> 1M -<--|-->- 1M <-> Gnd,
// #endif

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
    s32 temp;
    vs32 i;

    // Measure temp
    //    s16 *p=&ADC_ConvertedValue[1];     // intent; point to first of x results from same source - use a short name for it!
    // will not help reduce mains ripple because conversions are SO FAST!!
    // Calculate the mean value
    //    temp = (p[0]+p[1]+p[2]+p[3])/4;    // take avg of burst of 4 temp reads. may only help reject hi freq noise a bit
    // YRT 20081023 : add touchsreen adc channels for Primer2, buffer managed by DMA

    // Calculate the mean value
    temp = 0;
    for ( i = 0; i < ADC_NB_SAMPLES; i++ )
    {
        temp += ADC_ConvertedValue[1 + i*ADC_NB_CHANNELS];
    }
    temp = temp / ADC_NB_SAMPLES;

    temp = temp & 0xFFF;
    temp = ( temp * VDD_VOLTAGE_MV ) / 0x1000;  //finds mV
    temp = ((( 1400 - temp ) * 100000 ) / 448 ) + 25000; //gives approx temp x 1000 degrees C

    //Fahrenheit = 32 + 9 / 5 * Celsius
    if ( fTemperatureInFahrenheit )
    {
        temp = 32000 + ( 9 * temp ) / 5 ;
    }

    return temp / 100;
}

/*******************************************************************************
*
*                    UTIL_GetUsb
*
*******************************************************************************/
/**
*
*  Return the USB connexion state.
*
*  @return The USB connexion state.
*
**/
/******************************************************************************/
u8 UTIL_GetUsb( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   =  GPIO_USB_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init( GPIOx_USB, &GPIO_InitStructure );

    return ( GPIO_ReadInputDataBit( GPIOx_USB, GPIO_USB_PIN ) == Bit_SET );
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
*
**/
/******************************************************************************/
void UTIL_SetPll( enum eSpeed speed )
{
    /* Select PLL as system clock source */
    RCC_SYSCLKConfig( RCC_SYSCLKSource_HSI );

    /* Enable PLL */
    RCC_PLLCmd( DISABLE );

    if (( speed < SPEED_VERY_LOW ) || ( speed > SPEED_VERY_HIGH ) )
    {
        speed = SPEED_MEDIUM;
    }

    CurrentSpeed = speed;

    switch ( speed )
    {
        // 18 MHz
    case SPEED_VERY_LOW  :
        if ( !ClockBug )
            /* PLLCLK = (12MHz / 2) * 3 = 18 MHz */
            RCC_PLLConfig( RCC_PLLSource_HSE_Div2, RCC_PLLMul_3 );
        else
            /* PLLCLK = 36MHz * 1 = 36 MHz  !! */
            RCC_PLLConfig( RCC_PLLSource_HSE_Div2, RCC_PLLMul_2 );
        break;

        // 24MHz
    case SPEED_LOW       :
        if ( !ClockBug )
            /* PLLCLK = 12MHz * 2 = 24 MHz */
            RCC_PLLConfig( RCC_PLLSource_HSE_Div1, RCC_PLLMul_2 );
        else
            /* PLLCLK = 36MHz * 1 = 36 MHz !!  */
            RCC_PLLConfig( RCC_PLLSource_HSE_Div2, RCC_PLLMul_2 );
        break;

        // 36MHz
    default              :
        CurrentSpeed = SPEED_MEDIUM;
    case SPEED_MEDIUM    :
        if ( !ClockBug )
            /* PLLCLK = 12MHz * 3 = 36 MHz */
            RCC_PLLConfig( RCC_PLLSource_HSE_Div1, RCC_PLLMul_3 );
        else
            /* PLLCLK = 36MHz * 1 = 36 MHz */
            RCC_PLLConfig( RCC_PLLSource_HSE_Div2, RCC_PLLMul_2 );
        break;

        // 48MHz
    case SPEED_HIGH      :
        if ( !ClockBug )
            /* PLLCLK = 12MHz * 4 = 48 MHz */
            RCC_PLLConfig( RCC_PLLSource_HSE_Div1, RCC_PLLMul_4 );
        else
            /* PLLCLK = (36MHz / 2) * 3 = 54 MHz !! */
            RCC_PLLConfig( RCC_PLLSource_HSE_Div2, RCC_PLLMul_3 );
        break;

        // 72MHz
    case SPEED_VERY_HIGH :
        if ( !ClockBug )
            /* PLLCLK = 12MHz * 6 = 72 MHz */
            RCC_PLLConfig( RCC_PLLSource_HSE_Div1, RCC_PLLMul_6 );
        else
            /* PLLCLK = 36MHz * 2 = 72 MHz */
            RCC_PLLConfig( RCC_PLLSource_HSE_Div1, RCC_PLLMul_2 );
        break;
    }

    /* Enable PLL */
    RCC_PLLCmd( ENABLE );

    /* Wait till PLL is ready */
    while ( RCC_GetFlagStatus( RCC_FLAG_PLLRDY ) == RESET )
        { ; }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig( RCC_SYSCLKSource_PLLCLK );

    /* Wait till PLL is used as system clock source */
    while ( RCC_GetSYSCLKSource() != 0x08 )
        { ; }

    /* This function fills a RCC_ClocksTypeDef structure with the current frequencies
    of different on chip clocks (for debug purpose) */
    RCC_GetClocksFreq( &RCC_ClockFreq );

#ifdef PRIMER2
    {
        extern const u16 I2S_PrescalerVal[];
        // Set the prescaler values for the I2S (audio codec) according to the PLL
        *( u16* )SPI2_I2SPR = I2S_PrescalerVal[speed-1];
        *( u16* )SPI3_I2SPR = I2S_PrescalerVal[speed-1];
    }
#endif
}


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
NODEBUG bool UTIL_isBackupRegisterConfigured( void )
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
*                                UTIL_SetIrqHandler
*
*******************************************************************************/
/**
*
*  Redirect an IRQ handler.
*
*  @param[in]  Offs   Address in the NVIC table
*  @param[in]  pHDL   Pointer to the handler.
*
**/
/********************************************************************************/
void UTIL_SetIrqHandler( s32 Offs, tHandler pHDL )
{
#ifdef PRIMER1
    if (( Offs >= 8 ) && ( Offs < 0x100 ) )
#else
    if (( Offs >= 8 ) && ( Offs < 0x140 ) ) // KJ20090320 - Changed from 120
#endif
        *( tHandler* )( CIRCLEOS_RAM_BASE + Offs ) = pHDL;
}

/*******************************************************************************
*
*                                UTIL_GetIrqHandler
*
*******************************************************************************/
/**
*
*  Get the current IRQ handler.
*  Since (V1.6) the vector table is relocated in RAM, the vectors can be easily modified
*  by the applications.
*
*  @param[in]  Offs   Address in the NVIC table
*  @return  A pointer to the current handler.
*
**/
/********************************************************************************/
tHandler UTIL_GetIrqHandler( s32 Offs )
{
#ifdef PRIMER1
    if (( Offs >= 8 ) && ( Offs < 0x100 ) )
#else
    if (( Offs >= 8 ) && ( Offs < 0x140 ) ) // KJ20090320 - Changed from 120
#endif
        return *( tHandler* )( CIRCLEOS_RAM_BASE + Offs );
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
*           2 = PRIMER 2.
*
**/
/********************************************************************************/
u16 UTIL_GetPrimerType( void )
{
    u16 type = 2;

    /* GPIO Configuration: backlight control in input */
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE );
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOB, &GPIO_InitStructure );

    // Check if pull-up (Primer2), or pull_down LED0 (Primer 1)
    if ( GPIO_ReadInputDataBit( GPIOB, GPIO_Pin_8 ) == 1 )
        {
        type = 2;
        /* GPIO reconfiguration : BL TIM in Output */
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
        GPIO_Init( GPIOB, &GPIO_InitStructure );
        }
    else
        {
        type = 1;
        /* GPIO reconfiguration : LED0 in Output */
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
        GPIO_Init( GPIOB, &GPIO_InitStructure );
        }  
    
    return type;

}

/*******************************************************************************
*
*                                UTIL_GetAppAddress
*
*******************************************************************************/
/**
*
*  Get the address of an application in the FAT.
*
*  @param[in]  AppName   pointer on the application name.
*  @return     long      address of the application.
*
**/
/********************************************************************************/
uint_t UTIL_GetAppAddress( const u8* AppName )
{
    s32 i;

    for ( i = 0; i < MAXAPP; i++ )
    {
        tMenuItem*  curapp;
        long unsigned addr;

        if (( *ApplicationTable )[ -i ] == APP_VOID )
        {
            break;
        }
        else
        {
            addr   = ( long unsigned )( *ApplicationTable )[ -i ] ;
            addr &= 0x00FFFFFF;
            addr |= 0x08000000;
            curapp = ( tMenuItem* ) addr;
            if ( !strcmp( curapp->Text, AppName ) )
            {
                return addr;
            }
        }
    }

    return 0;
}

/*******************************************************************************
*
*                                UTIL_LoadApp
*
*******************************************************************************/
/**
*
*  Loads an application in the current memory space. The given application
*  code may be retrieved from uSD card (STM32) or from I2C EEPROM(STM8L).
*  @param[in] app is the index of the application to load.
*
**/
/********************************************************************************/
void UTIL_LoadApp( index_t app )
{
}


/*******************************************************************************
*
*                                my_strlen
*
*******************************************************************************/
/**
*  Behaves as the standard ANSI "strlen", return provided string length.
*
*  @param[in]  p The string to evaluate the length.
*
*  @return  The length of the provided string.
*
**/
/******************************************************************************/
NODEBUG len_t my_strlen( const u8* p )
{
    len_t len = 0;

    for ( ; *p; p++ )
    {
        len++;
    }

    return len;
}
