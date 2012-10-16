/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     mems_spe.c
* @brief    Mems Initialization and management
* @author   FL
* @date     07/2007
* @version  1.1
* @date     10/2007
* @version  1.5 various corrections reported by Ron Miller
* @version  4.0 Folder reorganization
* @date     05/2010
*
* @note     Platform = STM32
*
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private define ------------------------------------------------------------*/
#define RDOUTXL               0xE8  /*!< Multiple Read from OUTXL             */
#define WRCTRL_REG1           0x20  /*!< Single Write CTRL_REG                */
#define RDCTRL_REG1           0xA0  /*!< Single Read CTRL_REG                 */
#define RDID                  0x8F  /*!< Single Read WHO_AM_I                 */
#define DUMMY_BYTE            0xA5
#define MEMS_TESTING_DIVIDER  101
#define MARGIN                500
#define DELAY_REACT           20
#define MIN_REACT             15
#define DIV_REACT             10
#define GRAD_SHOCK            350000
#define HIGH                  0x01
#define LOW                   0x00
#define ACTIVE                0x00  /*!< ChipSelect line low         */
#define STDBY                 0x01  /*!< ChipSelect line high        */


/* Private variables ---------------------------------------------------------*/
tMEMS_Info                          MEMS_Info                        = {0};   // structure definition in circle.h
s32                                 TestingActive                    = 0;
s32                                 StartingFromResetOrShockCounter  = 1000;
s32                                 TimeCounterForDoubleClick        = 0;
s32                                 TimeLastShock                    = 0;
u32                                 Gradient2;

// Gradient
s32                                 GradX                            = 0;
s32                                 GradY                            = 0;
s32                                 GradZ                            = 0;

coord_t XInit = 0;
coord_t YInit = 0;
coord_t ZInit = 0;

/* Private function prototypes -----------------------------------------------*/
void MEMS_ChipSelect( u8 State );
u8   MEMS_SendByte( u8 byte );
u32  MEMS_ReadOutXY( void );
void MEMS_WakeUp( void );

/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
*
*                                MEMS_WakeUp
*
*******************************************************************************/
/**
*  Wake Up Mems.
*
**/
/******************************************************************************/
void MEMS_WakeUp( void )
{
    u8 reg_val;

    /* read RDCTRL_REG1 */

    /* Chip Select low */
    MEMS_ChipSelect( ACTIVE );

    /* Send "RDCTRL_REG1" instruction */
    MEMS_SendByte( RDCTRL_REG1 );

    reg_val = MEMS_SendByte( DUMMY_BYTE );

    /* Chip Select high */
    MEMS_ChipSelect( STDBY );

    /* SET P0:P1 to '11' */
    /* 0xC0 to wake up and 0x30 for full speed frequency (640 Hz). */
    reg_val = reg_val | 0xC0 | 0x30;

    /* Chip Select low */
    MEMS_ChipSelect( ACTIVE );

    /* Send "WRCTRL_REG1" instruction */
    MEMS_SendByte( WRCTRL_REG1 );
    MEMS_SendByte( reg_val );

    /* Chip Select high */
    MEMS_ChipSelect( STDBY );
}

/*******************************************************************************
*
*                                MEMS_ReadOutXY
*
*******************************************************************************/
/**
*  Reads X and Y Out.
*
*  @return An unsigned 32 bit word with the highest 16 bits containing the Y
*          and the lowest 16 bits the X.
*
**/
/******************************************************************************/
u32 MEMS_ReadOutXY( void )
{

    u8 OutXL;
    u8 OutXH;
    u8 OutYL;
    u8 OutYH;
    u8 OutZL;
    u8 OutZH;

    /* Chip Select low */
    MEMS_ChipSelect( ACTIVE );

    /* Send "RDOUTXL" instruction */
    MEMS_SendByte( RDOUTXL );

    /* Read a byte */
    OutXL = MEMS_SendByte( DUMMY_BYTE );

    /* Read a byte */
    OutXH = MEMS_SendByte( DUMMY_BYTE );

    /* Read a byte */
    OutYL = MEMS_SendByte( DUMMY_BYTE );

    /* Read a byte */
    OutYH = MEMS_SendByte( DUMMY_BYTE );

    /* Read a byte */
    OutZL = MEMS_SendByte( DUMMY_BYTE );

    /* Read a byte */
    OutZH = MEMS_SendByte( DUMMY_BYTE );

    MEMS_Info.OutX =  OutXL + ( OutXH << 8 );
    MEMS_Info.OutY =  OutYL + ( OutYH << 8 );
    MEMS_Info.OutZ =  OutZL + ( OutZH << 8 );

    /* Chip Select high */
    MEMS_ChipSelect( STDBY );

    MEMS_Info.OutX_F4 += ( MEMS_Info.OutX - ( MEMS_Info.OutX_F4 >> 2 ) ); // Filter on 4 values.
    MEMS_Info.OutY_F4 += ( MEMS_Info.OutY - ( MEMS_Info.OutY_F4 >> 2 ) ); // Filter on 4 values.
    MEMS_Info.OutZ_F4 += ( MEMS_Info.OutZ - ( MEMS_Info.OutZ_F4 >> 2 ) ); // Filter on 4 values.

    MEMS_Info.OutX_F16 += ( MEMS_Info.OutX - ( MEMS_Info.OutX_F16 >> 4 ) ); // Filter on 16 values.
    MEMS_Info.OutY_F16 += ( MEMS_Info.OutY - ( MEMS_Info.OutY_F16 >> 4 ) ); // Filter on 16 values.
    MEMS_Info.OutZ_F16 += ( MEMS_Info.OutZ - ( MEMS_Info.OutZ_F16 >> 4 ) ); // Filter on 16 values.

    MEMS_Info.OutX_F64 += ( MEMS_Info.OutX - ( MEMS_Info.OutX_F64 >> 6 ) ); // Filter on 64 values.
    MEMS_Info.OutY_F64 += ( MEMS_Info.OutY - ( MEMS_Info.OutY_F64 >> 6 ) ); // Filter on 64 values.
    MEMS_Info.OutZ_F64 += ( MEMS_Info.OutZ - ( MEMS_Info.OutZ_F64 >> 6 ) ); // Filter on 64 values.

    MEMS_Info.OutX_F256 += ( MEMS_Info.OutX - ( MEMS_Info.OutX_F256 >> 8 ) ); // Filter on 256 values.
    MEMS_Info.OutY_F256 += ( MEMS_Info.OutY - ( MEMS_Info.OutY_F256 >> 8 ) ); // Filter on 256 values.
    MEMS_Info.OutZ_F256 += ( MEMS_Info.OutZ - ( MEMS_Info.OutZ_F256 >> 8 ) ); // Filter on 256 values.

    return ( MEMS_Info.OutX + ( MEMS_Info.OutY << 16 ) );
}

/*******************************************************************************
*
*                                MEMS_ChipSelect
*
*******************************************************************************/
/**
*  Selects or deselects the MEMS device.
*
*  @param[in]  State Level to be applied on ChipSelect pin.
*
**/
/******************************************************************************/
void MEMS_ChipSelect( u8 State )
{
    /* Set High or low the chip select line */
    GPIO_WriteBit( GPIOX_MEMS_CS, GPIO_PIN_MEMS_CS, ( BitAction )State );
}

/*******************************************************************************
*
*                                MEMS_SendByte
*
*******************************************************************************/
/**
*  Sends a byte through the SPI interface and return the byte received from
*  the SPI bus.
*
*  @param[in]  byte The byte to send to the SPI interface.
*
*  @return The byte returned by the SPI bus.
*
**/
/******************************************************************************/
u8 MEMS_SendByte( u8 byte )
{
    /* Loop while DR register in not empty */
    while ( SPI_I2S_GetFlagStatus( SPIX_MEMS, SPI_I2S_FLAG_TXE ) == RESET );

    /* Send byte through the SPIx peripheral */
    SPI_I2S_SendData( SPIX_MEMS, byte );

    /* Wait to receive a byte */
    while ( SPI_I2S_GetFlagStatus( SPIX_MEMS, SPI_I2S_FLAG_RXNE ) == RESET );

    /* Return the byte read from the SPI bus */
    return SPI_I2S_ReceiveData( SPIX_MEMS );
}

/* Public functions for CircleOS ---------------------------------------------*/

/*******************************************************************************
*
*                                MEMS_Init
*
*******************************************************************************/
/**
*
*  Initializes the peripherals used by the SPI MEMS driver.
*
*  @attention  This function must <b>NOT</b> be called by the user.
*
**/
/******************************************************************************/
NODEBUG2 void MEMS_Init( void )
{
    SPI_InitTypeDef  SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable SPIx, GPIOX_MEMS_CTRL and GPIOX_MEMS_CS clocks */
    RCC_APBxPERIPH_MEMS_CLOCK_CMD( RCC_APBxPERIPH_SPIX_MEMS,  ENABLE );
    RCC_PERIPH_GPIO_CLOCK_CMD( RCC_APBxPERIPH_GPIOX_MEMS_CTRL, ENABLE );
    RCC_PERIPH_GPIO_CLOCK_CMD( RCC_APBxPERIPH_GPIOX_MEMS_CS, ENABLE );

#ifdef STM32L1XX_MD

    /* Configure MSD_SPI pins: SCK, MISO and MOSI */
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_MEMS_SCK | GPIO_PIN_MEMS_MISO | GPIO_PIN_MEMS_MOSI;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_Init( GPIOX_MEMS_CTRL, &GPIO_InitStructure );

    /* Connect PXx to SD_SPI_SCK, SD_SPI_MISO and  SD_SPI_MOSI*/
    GPIO_PinAFConfig(GPIOX_MEMS_CTRL, MEMS_SPI_SCK_SOURCE, MEMS_SPI_SCK_AF);
    GPIO_PinAFConfig(GPIOX_MEMS_CTRL, MEMS_SPI_MISO_SOURCE, MEMS_SPI_MISO_AF); 
    GPIO_PinAFConfig(GPIOX_MEMS_CTRL, MEMS_SPI_MOSI_SOURCE, MEMS_SPI_MOSI_AF);  
  
#else

    /* Configure SPI pins: SCK and MOSI */
    GPIO_InitStructure.GPIO_Pin   = GPIO_PIN_MEMS_SCK | GPIO_PIN_MEMS_MOSI;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init( GPIOX_MEMS_CTRL, &GPIO_InitStructure );

    /* Configure SPI pins: MISO */
    GPIO_InitStructure.GPIO_Pin   = GPIO_PIN_MEMS_MISO;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init( GPIOX_MEMS_CTRL, &GPIO_InitStructure );
    
#endif

    /* Configure Pxx as Output push-pull, used as MEMS Chip select */
    GPIO_InitStructure.GPIO_Pin   = GPIO_PIN_MEMS_CS;
#ifdef STM32L1XX_MD
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
#else
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
#endif
    GPIO_Init( GPIOX_MEMS_CS, &GPIO_InitStructure );

    /* SPI configuration */
    SPI_InitStructure.SPI_Direction           = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode                = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize            = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL                = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA                = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS                 = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler   = SPI_BaudRatePrescaler_16;
    SPI_InitStructure.SPI_FirstBit            = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial       = 7;

    SPI_Init( SPIX_MEMS, &SPI_InitStructure );

    /* Enable SPI  */
    SPI_Cmd( SPIX_MEMS, ENABLE );

    if ( MEMS_ReadID() != 0x3A )
    {
        s32 i;

        // Try to resynchronize
        for ( i = 0 ; i < 17 ; i++ )
        {
            /* Configure SPI2 pins: SCK, MISO and MOSI */
            GPIO_InitStructure.GPIO_Pin   = GPIO_PIN_MEMS_SCK | GPIO_PIN_MEMS_MOSI;
        #ifndef STM32L1XX_MD
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
            GPIO_Init( GPIOX_MEMS_CTRL, &GPIO_InitStructure );
        #else
            GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
        #endif
            GPIO_WriteBit( GPIOX_MEMS_CTRL, GPIO_PIN_MEMS_MOSI, HIGH );
            MEMS_ChipSelect( ACTIVE );

            GPIO_WriteBit( GPIOX_MEMS_CTRL, GPIO_PIN_MEMS_SCK, LOW );
            GPIO_WriteBit( GPIOX_MEMS_CTRL, GPIO_PIN_MEMS_SCK, HIGH );
            MEMS_ChipSelect( HIGH );

            /* Configure again PB. SCK as SPI2 pin */
            GPIO_InitStructure.GPIO_Pin   = GPIO_PIN_MEMS_SCK | GPIO_PIN_MEMS_MOSI;
         #ifndef STM32L1XX_MD
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
            GPIO_Init( GPIOX_MEMS_CTRL, &GPIO_InitStructure );
         #else
            GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
         #endif
         
            if ( MEMS_ReadID() == 0x3A )
            {
                break;
            }
        }

        if ( i == 17 )
        {
            DRAW_DisplayString( 1, 50, "Test MEM ID Failed", 18 );
            fIsStandAlone = 1;
        }
    }

    MEMS_ReadOutXY();

    MEMS_Info.OutX_F4 = MEMS_Info.OutX_F16 = MEMS_Info.OutX_F64 = MEMS_Info.OutX_F256 = MEMS_Info.OutX;
    MEMS_Info.OutY_F4 = MEMS_Info.OutY_F16 = MEMS_Info.OutY_F64 = MEMS_Info.OutY_F256 = MEMS_Info.OutY;
    MEMS_Info.OutZ_F4 = MEMS_Info.OutZ_F16 = MEMS_Info.OutZ_F64 = MEMS_Info.OutZ_F256 = MEMS_Info.OutZ;

    /* Init X and Y*/
    MEMS_GetPosition( &XInit, &YInit );

    /* Wake Up Mems*/
    MEMS_WakeUp();
}

/*******************************************************************************
*
*                                MEMS_Handler
*
*******************************************************************************/
/**
*
*  Called by the CircleOS scheduler to manage the MEMS. The Circle beeps if the
*  MEMS is shocked.
*
*  @attention  This function must <b>NOT</b> be called by the user.
*
**/
/******************************************************************************/
IRQ void MEMS_Handler( void )
{
    static s32 delta_time_ms = 0;

    if ( StartingFromResetOrShockCounter )
    {
        StartingFromResetOrShockCounter--;
    }
    TimeCounterForDoubleClick++;

    MEMS_ReadOutXY();

    // Evaluate gradients
    GradX = ( MEMS_Info.OutX_F16 >> 4 ) - MEMS_Info.OutX;
    GradY = ( MEMS_Info.OutY_F16 >> 4 ) - MEMS_Info.OutY;
    GradZ = ( MEMS_Info.OutZ_F16 >> 4 ) - MEMS_Info.OutZ;

    Gradient2 = ( s32 )GradX * ( s32 )GradX + ( s32 )GradY * ( s32 )GradY + ( s32 )GradZ * ( s32 )GradZ;

    Gradient2 *= freqTIM2[CurrentSpeed] ;
    Gradient2 /= freqTIM2[SPEED_MEDIUM] ;


    // MEMS is shocked, let's beep!
    if (( Gradient2 > GRAD_SHOCK ) && ( BUZZER_GetMode() == BUZZER_OFF ) && ( StartingFromResetOrShockCounter == 0 ) )
    {
        MEMS_Info.Shocked++;
        /*FL071007       = 1;
        Suggested by Bob Seabrook:  a further possiblity is to increment Shocked rather than just setting it
        So it can still be tested for non zero as before but one can  get more
        info from the s32 without extra cost. */

#define DELAY_BETWEEN_TWO_SHOCK     200
#define MAX_DELAY_FOR_DOUBLECLICK   800

        delta_time_ms = (( TimeCounterForDoubleClick - TimeLastShock ) * 1000 ) / freqTIM2[CurrentSpeed];

        if ( delta_time_ms > DELAY_BETWEEN_TWO_SHOCK )
        {
            if ( delta_time_ms < MAX_DELAY_FOR_DOUBLECLICK )
            {
                MEMS_Info.DoubleClick++;
//               TimeLastShock = TimeCounterForDoubleClick;    // YRT20090304
                BUZZER_SetMode( BUZZER_SHORTBEEP );
                StartingFromResetOrShockCounter = freqTIM2[CurrentSpeed]; //1s off
            }
            else
            {
//                TimeLastShock = TimeCounterForDoubleClick;   // YRT20090304
                delta_time_ms  = 0;
                StartingFromResetOrShockCounter = 0;
            }
            TimeLastShock = TimeCounterForDoubleClick;         // YRT20090304
        }
        else
        {
            ;//TimeLastShock = TimeCounterForDoubleClick;
        }
    }
}

/*******************************************************************************
*
*                                MEMS_ReadID
*
*******************************************************************************/
/**
*  Reads SPI chip identification.
*
*  @return The SPI chip identification.
*
**/
/******************************************************************************/
u8 MEMS_ReadID( void )
{
    u8 Temp = 0;

    /* Chip Select low */
    MEMS_ChipSelect( ACTIVE );

    /* Send "RDID" instruction */
    MEMS_SendByte( RDID );

    /* Read a byte from the MEMS */
    Temp = MEMS_SendByte( DUMMY_BYTE );

    /* Chip Select low */
    MEMS_ChipSelect( STDBY );

    return Temp;
}

/// @endcond

/* Public functions ----------------------------------------------------------*/

/*******************************************************************************
*
*                                MEMS_GetPosition
*
*******************************************************************************/
/**
*
*  Returns the current (relative) position of the Primer.
*  Only X-Y axis are considered here.
*
*  @param[out] pX    Current horizontal coordinate.
*  @param[out] pY    Current vertical coordinate.
*
*  @warning    The (0x0) point in on the low left corner.
*  @note       For absolute position information use MEMS_GetInfo()
*
**/
/******************************************************************************/
void MEMS_GetPosition( coord_t* pX, coord_t* pY )
{
    *pX = MEMS_Info.OutX - XInit;
    *pY = MEMS_Info.OutY - YInit;
}

/*******************************************************************************
*
*                                MEMS_GetRotation
*
*******************************************************************************/
/**
*
*  Returns current screen orientation.
*
*  @param[out]  pH12 Current screen orientation.
*
**/
/******************************************************************************/
void MEMS_GetRotation( Rotate_H12_V_Match_TypeDef* pH12 )
{
    s16 sX = MEMS_Info.OutX;
    s16 sY = MEMS_Info.OutY;

    if ((( sX <= -MARGIN ) && ( sY <= 0 ) && ( sX <= sY ) ) ||
            (( sX <= - MARGIN ) && ( sY > 0 ) && ( sX <= ( -sY ) ) ) )
    {
        // 1st case: x<0, |x|>y => H12 = V9
        *pH12 = V9;
    }
    else if ((( sY <= -MARGIN ) && ( sX <= 0 ) && ( sY <= sX ) ) ||
             (( sY <= -MARGIN ) && ( sX > 0 ) && ( sY <= ( -sX ) ) ) )
    {
        // 2nd case: y<0, |y|>x => H12 = V12
        *pH12 = V12;
    }
    else if ((( sX >= MARGIN ) && ( sY <= 0 ) && ( sX >= ( -sY ) ) ) ||
             (( sX >= MARGIN ) && ( sY > 0 ) && ( sX >= sY ) ) )
    {
        // 3rd case: x>0, |x|>y => H12=V3
        *pH12 = V3;
    }
    else if ((( sY >= MARGIN ) && ( sX <= 0 ) && ( sY >= ( -sX ) ) ) ||
             (( sY >= MARGIN ) && ( sX > 0 ) && ( sY >= sX ) ) )
    {
        // 4th case: y>0,  |y|>x => H12=V6
        *pH12 = V6;
    }
}

/*******************************************************************************
*
*                                MEMS_SetNeutral
*
*******************************************************************************/
/**
*
*  Set current position as "neutral position".
*
**/
/******************************************************************************/
void MEMS_SetNeutral( void )
{
    // Set Neutral position.
    MEMS_GetPosition( &XInit, &YInit );
}

/*******************************************************************************
*
*                                MEMS_GetInfo
*
*******************************************************************************/
/**
*
*  Return the current MEMS information (state, absolute position...).
*
*  @return  a pointer to tMEMS_Info
*
**/
/******************************************************************************/
tMEMS_Info* MEMS_GetInfo( void )
{
    return &MEMS_Info;
}
