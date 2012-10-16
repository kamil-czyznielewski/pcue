/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     lcd.c
* @brief    The LCD driver for the ILI9325.
* @author   YRT
* @date     09/2009
* @note     Platform = Open4 STM32C Daughter Board
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private define ------------------------------------------------------------*/
#define RAISO_HEIGHT     8
#define RAISO_WIDTH      64

/* Private variables ---------------------------------------------------------*/
CONST_DATA u8 raiso_bw[( RAISO_WIDTH * RAISO_HEIGHT ) / 8] =
{
#include "bmp\raiso_8_64.h"
};

#define BUTTERFLY_HEIGHT     48
#define BUTTERFLY_WIDTH      48
CONST_DATA u8 butterfly_bw[( BUTTERFLY_WIDTH * BUTTERFLY_HEIGHT ) / 8] =
{
#include "bmp\butterfly_48_48.h"
};

#define BUTTERFLYBABY_HEIGHT     24
#define BUTTERFLYBABY_WIDTH      24
CONST_DATA u8 butterflybaby_bw[( BUTTERFLYBABY_WIDTH * BUTTERFLYBABY_HEIGHT ) / 8] =
{
#include "bmp\butterfly_24_24.h"
};

#define ST_HEIGHT     32
#define ST_WIDTH      56
CONST_DATA u8 st_bw[( ST_WIDTH * ST_HEIGHT ) / 8] =
{
#include "bmp\st_32_56.h"
};

/* Vars for timer dedicated for lcd backlight */
static TIM_TimeBaseInitTypeDef      TIM_TimeBaseStructure;
static TIM_OCInitTypeDef            TIM_OCInitStructure;

/* Public variables ---------------------------------------------------------*/
uint_t Current_CCR_BackLightStart = DEFAULT_CCR_BACKLIGHTSTART;
const unsigned freqTIM2[ 6 ] = { 136, 136, 182, 273, 364,  546 } ;
u16 LCDStatus;

/* External variable ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/


/*******************************************************************************
*
*                                LCD_Interface_Init
*
*******************************************************************************/
/**
*
*  @attention  This function must <b>NOT</b> be called by the user.
*
**/
/******************************************************************************/
void LCD_Interface_Init( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /*-- GPIO Configuration ------------------------------------------------------*/

    /* LCD Data lines configuration */
    RCC_APB2PeriphClockCmd( GPIO_LCD_D_PERIPH, ENABLE );

    GPIO_InitStructure.GPIO_Pin =  LCD_DATA_PINS ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOx_D_LCD, &GPIO_InitStructure );

    /* CTRL pins configuration */
    RCC_APB2PeriphClockCmd( GPIO_LCD_CTRL_PERIPH, ENABLE );
    GPIO_InitStructure.GPIO_Pin = CtrlPin_RD | CtrlPin_WR | CtrlPin_RS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init( GPIOx_CTRL_LCD, &GPIO_InitStructure );

    /* Reset : configured as regular GPIO */
    RCC_APB2PeriphClockCmd( GPIO_LCD_RST_PERIPH, ENABLE );
    GPIO_InitStructure.GPIO_Pin = CtrlPin_RST;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init( GPIOx_RST_LCD, &GPIO_InitStructure );

    /* CS : configured as regular GPIO */
    RCC_APB2PeriphClockCmd( GPIO_LCD_CS_PERIPH, ENABLE );
    GPIO_InitStructure.GPIO_Pin = CtrlPin_CS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init( GPIOx_CS_LCD, &GPIO_InitStructure );

    GPIO_WriteBit( GPIOx_RST_LCD, CtrlPin_RST, Bit_SET );   /* Reset active at LOW */
    GPIO_WriteBit( GPIOx_CS_LCD, CtrlPin_CS, Bit_RESET );   /* CS active at LOW */
    GPIO_WriteBit( GPIOx_CTRL_LCD, CtrlPin_RD, Bit_SET );   /* RD = 1 */
    GPIO_WriteBit( GPIOx_CTRL_LCD, CtrlPin_WR, Bit_SET );   /* WR = 1 */

}


/*******************************************************************************
*
*                                LCD_BackLightConfig
*
*******************************************************************************/
/**
*  Setting of the PWM that drives the backlight intensity.
*
**/
/******************************************************************************/
   void LCD_BackLightConfig( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable GPIO clock  */
    RCC_APB2PeriphClockCmd( GPIO_LCD_BL_PERIPH, ENABLE );

    /* GPIO Configuration : TIM in Output */
    GPIO_InitStructure.GPIO_Pin   = GPIO_BACKLIGHT_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOx_BL_LCD, &GPIO_InitStructure );

    // Remaping TIM4 CH2 on PD13
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO, ENABLE );
    GPIO_PinRemapConfig( AFIO_MAPR_TIM4_REMAP, ENABLE );

    /* TIM Configuration -----------------------------------------------------*/
    /* TIMCLK = 12 MHz, Prescaler = 0x0 */

    /* Enable TIM clock */
    RCC_APB1PeriphClockCmd( TIM_LCD_BL_PERIPH, ENABLE );

    TIM_DeInit( TIM_LCD_BL );
    TIM_TimeBaseStructInit( &TIM_TimeBaseStructure );
    TIM_OCStructInit( &TIM_OCInitStructure );

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period          = 0xFFFF;
    TIM_TimeBaseStructure.TIM_Prescaler       = 0x00;
    TIM_TimeBaseStructure.TIM_ClockDivision   = 0x0;
    TIM_TimeBaseStructure.TIM_CounterMode     = TIM_CounterMode_Up;
    TIM_TimeBaseInit( TIM_LCD_BL, &TIM_TimeBaseStructure );

    /* Output Compare Toggle Mode configuration */
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;   /* FWLib v2.0*/
    TIM_OCInitStructure.TIM_Pulse       = Current_CCR_BackLightStart;

    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OCxInit( TIM_LCD_BL, &TIM_OCInitStructure );

    TIM_OCxPreloadConfig( TIM_LCD_BL, TIM_OCPreload_Disable );

    TIM_ARRPreloadConfig( TIM_LCD_BL, ENABLE );

    /* Go !!!*/
    TIM_Cmd( TIM_LCD_BL, ENABLE );

}

/*******************************************************************************
*
*                               LCD_DataLinesConfig
*
*******************************************************************************/
/**
*  Configure data lines D0~D7 in Input Floating mode for read from LCD or in
*  Output Push-Pull mode for write on LCD
*
*  @param[in]  Mode Specifies the configuration mode for data lines D0~D7.
*                @n @c Input: configure in Input Floating mode
*                @n @c Output: configure in Output Push-Pull mode
*
**/
/******************************************************************************/
   void LCD_DataLinesConfig( DataConfigMode_TypeDef Mode )
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   =  LCD_DATA_PINS;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    if ( Mode == Input )
    {
        /* Configure D0~D7 lines as Input */
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    }
    else
    {
        /* Configure D0~D7 lines in Output Push-Pull mode */
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    }

    GPIO_Init( GPIOx_D_LCD, &GPIO_InitStructure );
}

/*******************************************************************************
*
*                                LCD_DataLinesWrite
*
*******************************************************************************/
/**
*  Write a value on D0~D7
*
*  @param[in]  GPIOx    GPIO port to write on.
*  @param[in]  PortVal  The value to write. Only the lowest 8 bits are taken into
*                       account.
*
**/
/******************************************************************************/
void LCD_DataLinesWrite( GPIO_TypeDef* GPIOx, u32 PortVal )
{
    // Write only bits 7..14
    GPIOx->ODR = (( GPIOx->ODR ) & ~LCD_DATA_PINS ) | (( u8 )PortVal << 7 );
}

/*******************************************************************************
*
*                                LCD_Write_Reg
*
*******************************************************************************/
/**
*  Write a LCD register.
*
**/
/******************************************************************************/
inline void LCD_Write_Reg( u8 regadd, u16 value )
{
    //=== Set index of the register
//    LCD_SendLCDCmd(regadd);

    /* Start the LCD send data sequence for the first byte = 0 */
    GPIOx_CTRL_LCD->BRR = CtrlPin_RS;       /* RS = 0 */

    // Write datas (only bits 7..14)
    GPIOx_D_LCD->ODR = (( GPIOx_D_LCD->ODR ) & ~LCD_DATA_PINS );
    GPIOx_CTRL_LCD->BRR = CtrlPin_WR;       /* WR = 0 */
    GPIOx_CTRL_LCD->BSRR = CtrlPin_WR;      /* WR = 1 */

    /* Start the LCD send data sequence for the second byte */

    // Write data (only bits 7..14)
    GPIOx_D_LCD->ODR = (( GPIOx_D_LCD->ODR ) & ~LCD_DATA_PINS ) | (( u8 )regadd << 7 );
    GPIOx_CTRL_LCD->BRR = CtrlPin_WR;       /* WR = 0 */
    GPIOx_CTRL_LCD->BSRR = CtrlPin_WR;      /* WR = 1 */


    //== Write into the register (2 x 8 bits transfers, MSB first)
//    LCD_SendLCDData(value >> 8);
    /* Start the LCD send data sequence */
    GPIOx_CTRL_LCD->BSRR = CtrlPin_RS;          /* RS = 1 */
    GPIOx_CTRL_LCD->BRR = CtrlPin_WR;           /* WR = 0 */

    /* Write data to the LCD */
    GPIOx_D_LCD->ODR = (( GPIOx_D_LCD->ODR ) & ~LCD_DATA_PINS ) | (( u8 )( value >> 8 ) << 7 );
    GPIOx_CTRL_LCD->BSRR = CtrlPin_WR;          /* WR = 1 */

//    LCD_SendLCDData(value & 0xFF);
    /* Start the LCD send data sequence */
    GPIOx_CTRL_LCD->BSRR = CtrlPin_RS;          /* RS = 1 */
    GPIOx_CTRL_LCD->BRR = CtrlPin_WR;           /* WR = 0 */

    /* Write data to the LCD */
    GPIOx_D_LCD->ODR = (( GPIOx_D_LCD->ODR ) & ~LCD_DATA_PINS ) | (( u8 )( value & 0xFF ) << 7 );
    GPIOx_CTRL_LCD->BSRR = CtrlPin_WR;          /* WR = 1 */
}

/*******************************************************************************
*
*                                LCD_Read_Reg
*
*******************************************************************************/
/**
*  Read a LCD register.
*
**/
/******************************************************************************/
inline u16 LCD_Read_Reg( u8 regadd )
{
    u16 temp1, temp2;

    // Set index of the register
    LCD_SendLCDCmd( regadd );

    // Read the value of the register
    temp1 = LCD_ReadLCDData() << 8;
    temp2 = LCD_ReadLCDData();

    return ( temp1 | temp2 );
}

/*******************************************************************************
*
*                                LCD_Reset
*
*******************************************************************************/
/**
*  Hard reset of the LCD.
*
**/
/******************************************************************************/
  void  LCD_Reset( void )
{
    GPIO_WriteBit( GPIOx_RST_LCD, CtrlPin_RST, Bit_RESET );     /* Reset active at LOW */
    Delayms( 300 );
    GPIO_WriteBit( GPIOx_RST_LCD, CtrlPin_RST, Bit_SET );       /* Reset active at LOW */
    Delayms( 100 );

    return;
}


/*******************************************************************************
*
*                                LCD_BackLightChange
*
*******************************************************************************/
/**
*  Modify the PWM rate.
*
**/
/******************************************************************************/
void LCD_BackLightChange( void )
{
    /* Output Compare Toggle Mode configuration */
    TIM_OCInitStructure.TIM_Pulse = Current_CCR_BackLightStart;

    TIM_OCxInit( TIM_LCD_BL, &TIM_OCInitStructure );
}


/*******************************************************************************
*
*                                LCD_SetLogoBW
*
*******************************************************************************/
/**
*
*  Draw the butterfly logo
*
**/
/******************************************************************************/
void LCD_SetLogoBW( void )
{
    TextColor = RGB_BLACK;
    DRAW_SetImageBW( butterfly_bw, 10 + 5 + BUTTERFLYBABY_WIDTH, ( Screen_Height - BUTTERFLY_WIDTH ) / 2 + BUTTERFLYBABY_HEIGHT, BUTTERFLY_WIDTH, BUTTERFLY_HEIGHT );
    DRAW_SetImageBW( butterflybaby_bw, 10 + 2 + BUTTERFLYBABY_WIDTH, ( Screen_Height - BUTTERFLYBABY_WIDTH ) / 2 - 10, BUTTERFLYBABY_WIDTH, BUTTERFLYBABY_HEIGHT );
    DRAW_SetImageBW( butterflybaby_bw, 10, 10 + ( Screen_Height - BUTTERFLYBABY_WIDTH ) / 2, BUTTERFLYBABY_WIDTH, BUTTERFLYBABY_HEIGHT );

    TextColor = RGB_LIGHTBLUE;
    DRAW_SetImageBW( st_bw, 10, ( Screen_Height - ST_HEIGHT ) - 20, ST_WIDTH, ST_HEIGHT );

    TextColor = RGB_RED;
    DRAW_SetImageBW( raiso_bw, ( Screen_Width - RAISO_WIDTH ) / 2, ( Screen_Height - RAISO_HEIGHT - 30 ), RAISO_WIDTH, RAISO_HEIGHT );

    TextColor = RGB_BLUE;
    BGndColor = RGB_WHITE;
    DRAW_DisplayStringWithMode( 0, 26, "Welcome to STM32C Primer", ALL_SCREEN, NORMAL_TEXT, CENTER );
    TextColor = RGB_BLACK;
}


/*******************************************************************************
*
*                                LCD_Scroll
*
*******************************************************************************/
/**
*
*  Scroll the screen each line to the top
*
**/
/******************************************************************************/
void LCD_Scroll( u8 Ascii, coord_t PosCurX, coord_t* PosCurY,
                         coord_t RightMarginX, coord_t LeftMarginX, coord_t HighMarginY, coord_t LowMarginY,
                         color_t bGndColor, color_t textColor, mag_t CharMagniCoeff )
{
    s32 Nblines, width, i, y;
    extern u16 bmpTmp [LCD_DMA_SIZE];

    InitListDMA();
    Nblines = ( LowMarginY - *PosCurY ) / DELTA_Y;
    width = RightMarginX - LeftMarginX;
    for ( i = 0; i < Nblines ; i++ )
    {
        for ( y = HighMarginY - DELTA_Y; y >= LowMarginY; y -= DELTA_Y )
        {
            LCD_RectRead( LeftMarginX, y - DELTA_Y, width, DELTA_Y, ( u8* ) &bmpTmp );
            DRAW_SetImage( bmpTmp, LeftMarginX, y, width, DELTA_Y );
//            LIST_LCD_RectRead( LeftMarginX, y - DELTA_Y, width, DELTA_Y ); // No multiread data with this controller
//            LIST_DRAW_SetImage( LeftMarginX, y, width, DELTA_Y );
        }
        LCD_FillRect( LeftMarginX, LowMarginY, width, DELTA_Y, bGndColor );
    }
    *PosCurY = LowMarginY;

    // Display the selected bitmap according to the provided ASCII character.
    LCD_DisplayChar( PosCurX, *PosCurY, Ascii, textColor, bGndColor, CharMagniCoeff );

}


/*******************************************************************************
*
*                                LCD_SendLCDCmd_RAM_Access
*
*******************************************************************************/
/**
*   Call LCD_SendLCDCmd function for RAM access
*
*
**/
/******************************************************************************/
void LCD_SendLCDCmd_RAM_Access( void )
{
//    LCD_SendLCDCmd(ILI9325_GRAM_DATA);
    // Set index of the register

    /* Start the LCD send data sequence for the first byte = 0 */
    GPIOx_CTRL_LCD->BRR = CtrlPin_RS;       /* RS = 0 */

    // Write datas (only bits 7..14)
    GPIOx_D_LCD->ODR = (( GPIOx_D_LCD->ODR ) & ~LCD_DATA_PINS );
    GPIOx_CTRL_LCD->BRR = CtrlPin_WR;       /* WR = 0 */
    GPIOx_CTRL_LCD->BSRR = CtrlPin_WR;      /* WR = 1 */

    /* Start the LCD send data sequence for the second byte */

    // Write data (only bits 7..14)
    GPIOx_D_LCD->ODR = (( GPIOx_D_LCD->ODR ) & ~LCD_DATA_PINS ) | (( u8 )ILI9325_GRAM_DATA << 7 );
    GPIOx_CTRL_LCD->BRR = CtrlPin_WR;       /* WR = 0 */
    GPIOx_CTRL_LCD->BSRR = CtrlPin_WR;      /* WR = 1 */
}

/*******************************************************************************
*
*                                LCD_SendLCDCmd_RAM_Access_End
*
*******************************************************************************/
/**
*   To be called at the end of several bytes transmission
*   (Present only for LCD SPI mode compatibility)
*
**/
/******************************************************************************/
  void LCD_SendLCDCmd_RAM_Access_End( void )
{
}

/*******************************************************************************
*
*                                LCD_SendLCDCmd_Rotation_Access
*
*******************************************************************************/
/**
*   Call LCD_SendLCDCmd function for rotation register access
*
*
**/
/******************************************************************************/
void LCD_SendLCDCmd_Rotation_Access( void )
{
//    LCD_SendLCDCmd(ILI9325_ENTRY_MOD);
    // Set index of the register

    /* Start the LCD send data sequence for the first byte = 0 */
    GPIOx_CTRL_LCD->BRR = CtrlPin_RS;       /* RS = 0 */

    // Write datas (only bits 7..14)
    GPIOx_D_LCD->ODR = (( GPIOx_D_LCD->ODR ) & ~LCD_DATA_PINS );
    GPIOx_CTRL_LCD->BRR = CtrlPin_WR;       /* WR = 0 */
    GPIOx_CTRL_LCD->BSRR = CtrlPin_WR;      /* WR = 1 */

    /* Start the LCD send data sequence for the second byte */

    // Write data (only bits 7..14)
    GPIOx_D_LCD->ODR = (( GPIOx_D_LCD->ODR ) & ~LCD_DATA_PINS ) | (( u8 )ILI9325_ENTRY_MOD << 7 );
    GPIOx_CTRL_LCD->BRR = CtrlPin_WR;       /* WR = 0 */
    GPIOx_CTRL_LCD->BSRR = CtrlPin_WR;      /* WR = 1 */
}

/*******************************************************************************
*
*                                LCD_PixelRead
*
*******************************************************************************/
/**
*
*  Save the pixels of a rectangle part of the LCD into a RAM variable.
*
*  @param[out] ptbmp      Address of the pointer to store the read data into.
*
*  @warning    One pixel weights 2 bytes.
*  @warning    The (0x0) point in on the low left corner.
*
**/
/******************************************************************************/
inline void LCD_PixelRead( u8** ptbmp )
{
    u8 temp1, temp2, blue, red;

    // Send LCD RAM read command
//    LCD_SendLCDCmd(ILI9325_GRAM_DATA);
    /* Start the LCD send data sequence for the first byte = 0 */
    GPIOx_CTRL_LCD->BRR = CtrlPin_RS;       /* RS = 0 */
    // Write datas (only bits 7..14)
    GPIOx_D_LCD->ODR = (( GPIOx_D_LCD->ODR ) & ~LCD_DATA_PINS );
    GPIOx_CTRL_LCD->BRR = CtrlPin_WR;       /* WR = 0 */
    GPIOx_CTRL_LCD->BSRR = CtrlPin_WR;      /* WR = 1 */
    /* Start the LCD send data sequence for the second byte */
    // Write data (only bits 7..14)
    GPIOx_D_LCD->ODR = (( GPIOx_D_LCD->ODR ) & ~LCD_DATA_PINS ) | ( ILI9325_GRAM_DATA << 7 );
    GPIOx_CTRL_LCD->BRR = CtrlPin_WR;       /* WR = 0 */
    GPIOx_CTRL_LCD->BSRR = CtrlPin_WR;      /* WR = 1 */

    /* Configure Data lines as Input */
//    LCD_DataLinesConfig(Input);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   =  LCD_DATA_PINS;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init( GPIOx_D_LCD, &GPIO_InitStructure );

    /* Start the LCD read data sequence */
    GPIOx_CTRL_LCD->BSRR = CtrlPin_RS;      /* RS = 1 */

    // Two first read bytes are dummy !
    GPIOx_CTRL_LCD->BRR = CtrlPin_RD;       /* RD = 0 */
    temp1 = ( u8 )(( GPIOx_D_LCD->IDR & LCD_DATA_PINS ) >> 7 );
    GPIOx_CTRL_LCD->BSRR = CtrlPin_RD;      /* RD = 1 */

    GPIOx_CTRL_LCD->BRR = CtrlPin_RD;       /* RD = 0 */
    temp1 = ( u8 )(( GPIOx_D_LCD->IDR & LCD_DATA_PINS ) >> 7 );
    GPIOx_CTRL_LCD->BSRR = CtrlPin_RD;      /* RD = 1 */

    // Read the two bytes of the pixel
    // Read data (only bits 7..14)
    GPIOx_CTRL_LCD->BRR = CtrlPin_RD;       /* RD = 0 */
    delay( 1 );                             // if no delay reading pb !
    temp1 = ( u8 )((( GPIOx_D_LCD->IDR & LCD_DATA_PINS ) >> 7 ) & 0xFF );
    GPIOx_CTRL_LCD->BSRR = CtrlPin_RD;      /* RD = 1 */

    // Read data (only bits 7..14)
    GPIOx_CTRL_LCD->BRR = CtrlPin_RD;       /* RD = 0 */
    delay( 1 );                             // if no delay reading pb !
    temp2 = ( u8 )((( GPIOx_D_LCD->IDR & LCD_DATA_PINS ) >> 7 ) & 0xFF );
    GPIOx_CTRL_LCD->BSRR = CtrlPin_RD;      /* RD = 1 */

    // Convert in Primer RGB (the LCD send BGR data)
    blue = ( temp1 & 0xF8 ) >> 3;
    red  = ( temp2 & 0x1F ) << 3;

    *( *ptbmp )   = ( temp1 & 0x07 ) | red;
    *( *ptbmp + 1 ) = ( temp2 & 0xE0 ) | blue;
    *ptbmp += 2;

    /* Reconfigure lines into ouputs */
//    LCD_DataLinesConfig( Output );
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init( GPIOx_D_LCD, &GPIO_InitStructure );

}

/*******************************************************************************
*
*                                LCD_SendLCDData16
*
*******************************************************************************/
/**
*
*  Send two data bytes to the LCD.
*
*  @param[in]  Data_h  an unsigned byte containing the high byte of data to send to the LCD.
*  @param[in]  Data_l  an unsigned byte containing the low byte of data to send to the LCD.
*
*
**/
/******************************************************************************/
void LCD_SendLCDData16( u8 Data_h, u8 Data_l )
{
    /* Start the LCD send data sequence */
    GPIOx_CTRL_LCD->BSRR = CtrlPin_RS;          /* RS = 1 */
    GPIOx_CTRL_LCD->BRR = CtrlPin_WR;           /* WR = 0 */

    /* Write first data to the LCD */
    GPIOx_D_LCD->ODR = (( GPIOx_D_LCD->ODR ) & ~LCD_DATA_PINS ) | (( u8 )Data_h << 7 );
    GPIOx_CTRL_LCD->BSRR = CtrlPin_WR;          /* WR = 1 */

    GPIOx_CTRL_LCD->BRR = CtrlPin_WR;           /* WR = 0 */

    /* Write second data to the LCD */
    GPIOx_D_LCD->ODR = (( GPIOx_D_LCD->ODR ) & ~LCD_DATA_PINS ) | (( u8 )Data_l << 7 );
    GPIOx_CTRL_LCD->BSRR = CtrlPin_WR;          /* WR = 1 */
}

/// @endcond

/* Public functions ----------------------------------------------------------*/

/*******************************************************************************
*
*                                LCD_SendLCDCmd
*
*******************************************************************************/
/**
*
*  Send on command byte to the LCD.
*
*  @param[in]  Cmd   An u8 containing the user command to send to the LCD.
*
**/
/******************************************************************************/
void LCD_SendLCDCmd( u8 Cmd )
{
    // Set index of the register

    /* Start the LCD send data sequence for the first byte = 0 */
    GPIOx_CTRL_LCD->BRR = CtrlPin_RS;       /* RS = 0 */

    // Write datas (only bits 7..14)
    GPIOx_D_LCD->ODR = (( GPIOx_D_LCD->ODR ) & ~LCD_DATA_PINS );
    GPIOx_CTRL_LCD->BRR = CtrlPin_WR;       /* WR = 0 */
    GPIOx_CTRL_LCD->BSRR = CtrlPin_WR;      /* WR = 1 */

    /* Start the LCD send data sequence for the second byte */

    // Write data (only bits 7..14)
    GPIOx_D_LCD->ODR = (( GPIOx_D_LCD->ODR ) & ~LCD_DATA_PINS ) | (( u8 )Cmd << 7 );
    GPIOx_CTRL_LCD->BRR = CtrlPin_WR;       /* WR = 0 */
    GPIOx_CTRL_LCD->BSRR = CtrlPin_WR;      /* WR = 1 */
}

/*******************************************************************************
*
*                                LCD_SendLCDData
*
*******************************************************************************/
/**
*
*  Send one data byte to the LCD.
*
*  @param[in]  Data  An unsigned character containing the data to send to the LCD.
*  @pre        An LCD_SendLCDCmd was done with a command waiting for data.
*
*
**/
/******************************************************************************/
void LCD_SendLCDData( u8 Data )
{
    /* Start the LCD send data sequence */
    GPIOx_CTRL_LCD->BSRR = CtrlPin_RS;          /* RS = 1 */
    GPIOx_CTRL_LCD->BRR = CtrlPin_WR;           /* WR = 0 */

    /* Write data to the LCD */
    GPIOx_D_LCD->ODR = (( GPIOx_D_LCD->ODR ) & ~LCD_DATA_PINS ) | (( u8 )Data << 7 );
    GPIOx_CTRL_LCD->BSRR = CtrlPin_WR;          /* WR = 1 */
}


/***********************************************************************************
*
*                                LCD_ReadLCDData
*
************************************************************************************/
/**
*
*  Read one data byte from the LCD.
*
*  @return     An unsigned 32 bit word containing the data returned by a LCD command.
*  @pre        An LCD_SendLCDCmd was done with a command returning data.
*
**/
/********************************************************************************/
u32 LCD_ReadLCDData( void )
{
    u32 LCDData = 0;

    /* Configure Data lines as Input */
//    LCD_DataLinesConfig(Input);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   =  LCD_DATA_PINS;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init( GPIOx_D_LCD, &GPIO_InitStructure );

    /* Start the LCD read data sequence */
    GPIOx_CTRL_LCD->BSRR = CtrlPin_RS;      /* RS = 1 */
    GPIOx_CTRL_LCD->BRR = CtrlPin_RD;       /* RD = 0 */
    delay( 1 );

    // Read data (only bits 7..14)
    LCDData = ((GPIOx_D_LCD->IDR & LCD_DATA_PINS ) & LCD_DATA_PINS ) >> 7;

    GPIOx_CTRL_LCD->BSRR = CtrlPin_RD;      /* RD = 1 */

    /* Reconfigure lines into ouputs */
//    LCD_DataLinesConfig( Output );
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init( GPIOx_D_LCD, &GPIO_InitStructure );

    return LCDData;
}

/*******************************************************************************
*
*                                LCD_DrawPixel
*
*******************************************************************************/
/**
*
*  Draw a pixel on the LCD with the provided color.
*
*  @param[in]  XPos     The horizontal coordinate of the pixel.
*  @param[in]  YPos     The vertical coordinate of the pixel.
*  @param[in]  Color    The RGB color to draw the pixel with.
*
*  @warning    The (0x0) point in on the low left corner.
*
**/
/******************************************************************************/
void LCD_DrawPixel( coord_t XPos, coord_t YPos, color_t Color )
{
    /* Select LCD screen area. */
    LCD_SetRect_For_Cmd( XPos, YPos, 1, 1 );

    /* Send LCD RAM write command. */
    LCD_SendLCDCmd( ILI9325_GRAM_DATA );

    /* Draw pixel.*/
    LCD_SendLCDData( Color & 0xFF );
    LCD_SendLCDData( Color >> 8 );

}

