/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     lcd_spe.c
* @brief    Hardware specific LCD driver for the ST7637 and  ST7732.
* @author   YRT
* @date     09/2009
* @Note     Split from lcd.c
**/
/******************************************************************************

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private define ------------------------------------------------------------*/
#ifdef PRIMER1

#define RAISO_HEIGHT     8
#define RAISO_WIDTH      64
u8 raiso_bw [( RAISO_WIDTH* RAISO_HEIGHT )/8] =
{
#include "bmp\raiso_8_64.h"
};
#define BUTTERFLY_HEIGHT     48
#define BUTTERFLY_WIDTH      48
static const u8 butterfly_bw [( BUTTERFLY_WIDTH* BUTTERFLY_HEIGHT )/8] =
{
#include "bmp\butterfly_48_48.h"
};

#define BUTTERFLYBABY_HEIGHT     24
#define BUTTERFLYBABY_WIDTH      24
static const u8 butterflybaby_bw [( BUTTERFLYBABY_WIDTH* BUTTERFLYBABY_HEIGHT )/8] =
{
#include "bmp\butterfly_24_24.h"
};

#define ST_HEIGHT     32
#define ST_WIDTH      56
static const u8 st_bw [( ST_WIDTH* ST_HEIGHT )/8] =
{
#include "bmp\st_32_56.h"
};

#endif // PRIMER1

/* Private variables ---------------------------------------------------------*/
/* Vars for timer dedicated for lcd backlight */
static TIM_TimeBaseInitTypeDef      TIM_TimeBaseStructure;
static TIM_OCInitTypeDef            TIM_OCInitStructure;

/* External variable ---------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/
uint_t Current_CCR_BackLightStart = DEFAULT_CCR_BACKLIGHTSTART;
const unsigned freqTIM2[ 6 ] = { 136, 136, 182, 273, 364,  546 } ;

#ifdef PRIMER1
u8 OrientationOffsetX [] = { +4 /* V12*/, 0 /* V3*/, 0 /* V6*/, +4 /* V9*/ , +4 /* V12BMP*/, 0 /* V3*/, 0 /* V6*/, +4 /* V9*/ };
u8 OrientationOffsetY [] = { 0 /* V12*/, 0 /* V3*/, +4 /* V6*/, +4 /* V9*/ , 0  /* V12BMP*/, 0 /* V3*/, +4 /* V6*/, +4 /* V9*/ };
#endif
#ifdef PRIMER2
u8 OrientationOffsetX [] = { +0   /* V12*/,  0    /* V3*/,    0/* V6*/,  +32   /* V9*/ , +0 /*V12BMP*/,  0    /* V3BMP*/,  0 /* V6BMP*/,  32   /* V9BMP*/};
u8 OrientationOffsetY [] = { 0    /* V12*/,  0    /* V3*/,  +32/* V6*/,  0     /* V9*/ , +0 /*V12BMP*/,  0    /* V3BMP*/,  32   /* V6BMP*/, 0 /* V9BMP*/};
#endif

/* Private function prototypes -----------------------------------------------*/
#ifdef PRIMER1
static void LCD_7637_Controller( void );
#endif /*PRIMER1*/

#ifdef PRIMER2
static void LCD_ST7732S_Controller_init( void );
#endif /*PRIMER2*/

static void LCD_CtrlLinesWrite( GPIO_TypeDef* GPIOx, u32 CtrlPins, BitAction BitVal );

/* Private functions ---------------------------------------------------------*/

#ifdef PRIMER1
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
*                @n @c Output: configure in Output Push-Pul mode
*
**/
/******************************************************************************/
static void LCD_DataLinesConfig( DataConfigMode_TypeDef Mode )
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
#endif /*PRIMER1*/

#ifdef PRIMER1
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
static void LCD_DataLinesWrite( GPIO_TypeDef* GPIOx, u32 PortVal )
{
    // Write only the lowest 8 bits!
    GPIOx->ODR = (( GPIOx->ODR ) & 0xFF00 ) | ( u8 )PortVal;
}
#endif /*PRIMER1*/


#ifdef PRIMER1
/*******************************************************************************
*
*                                LCD_CtrlLinesConfig
*
*******************************************************************************/
/**
*  Configure control lines in Output Push-Pull mode.
*
**/
/******************************************************************************/
static void LCD_CtrlLinesConfig( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   =  LCD_CTRL_PINS;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;

    GPIO_Init( GPIOx_CTRL_LCD, &GPIO_InitStructure );

    GPIO_InitStructure.GPIO_Pin   =  CtrlPin_CS;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;

    GPIO_Init( GPIOx_CS_LCD, &GPIO_InitStructure );

    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_RS,  Bit_SET );    /* RS = 1   */
    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_RD,  Bit_SET );    /* RD = 1   */
    LCD_CtrlLinesWrite( GPIOx_CS_LCD,   CtrlPin_CS,  Bit_SET );    /* CS = 1   */
    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_WR,  Bit_SET );    /* WR = 1   */
    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_RST, Bit_RESET );  /* RST = 0  */
}
#endif /*PRIMER1*/


#ifdef PRIMER1
/*******************************************************************************
*
*                                LCD_CtrlLinesWrite
*
*******************************************************************************/
/**
*  Set or reset control lines.
*
*  @param[in]  GPIOx       Where x can be 0, 1 or 2 to select the GPIO peripheral.
*  @param[in]  CtrlPins    The Control line.
*  @param[in]  BitVal
*
**/
/******************************************************************************/
static void LCD_CtrlLinesWrite( GPIO_TypeDef* GPIOx, u32 CtrlPins, BitAction BitVal )
{
    /* Set or Reset the control line */
    GPIO_WriteBit( GPIOx, CtrlPins, BitVal );
}

#endif /*PRIMER1*/


/*******************************************************************************
*
*                                LCD_CheckLCDStatus
*
*******************************************************************************/
/**
*  Check whether LCD LCD is busy or not.
*
**/
/******************************************************************************/
static void LCD_CheckLCDStatus( void )
{
    u8 ID1;
    u8 ID2;
    u8 ID3;

#ifdef PRIMER1
    LCD_SendLCDCmd( ST7637_RDDID );

    /* Configure Data lines as Input */
    LCD_DataLinesConfig( Input );

    /* Start the LCD send data sequence */
    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_RS, Bit_RESET );     /* RS = 0 */
    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_RD, Bit_RESET );     /* RD = 0 */
    LCD_CtrlLinesWrite( GPIOx_CS_LCD,   CtrlPin_CS, Bit_RESET );     /* CS = 0 */
    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_WR, Bit_SET );       /* WR = 1 */

    /* Read data to the LCD */
    GPIO_ReadInputData( GPIOx_D_LCD );
    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_RD, Bit_SET );       /* RD = 1 */
    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_RD, Bit_RESET );     /* RD = 0 */

    ID1 = GPIO_ReadInputData( GPIOx_D_LCD );

    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_RD, Bit_SET );       /* RD = 1 */
    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_RD, Bit_RESET );     /* RD = 0 */

    ID2 = GPIO_ReadInputData( GPIOx_D_LCD );

    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_RD, Bit_SET );       /* RD = 1 */
    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_RD, Bit_RESET );     /* RD = 0 */

    ID3 = GPIO_ReadInputData( GPIOx_D_LCD );

    LCD_DataLinesConfig( Output );
#endif /*PRIMER1*/


#ifdef PRIMER2
    /* send command RDDID*/
    LCD_SendLCDCmd( ST7732_RDDID );
    /* Read LCD device IDs*/
    LCD_ReadLCDData();
    LCD_ReadLCDData();
    LCD_ReadLCDData();
#endif /*PRIMER2*/
}


#ifdef PRIMER1
/*******************************************************************************
*
*                                LCD_7637_Controller
*
*******************************************************************************/
/**
*  Initialization of the controller registers.
*
*  @note See ST7637.PDF for more information.
*
**/
/******************************************************************************/
static void LCD_7637_Controller( void )
{
    /** Apply hardware reset **/
    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_RST, Bit_SET );    /* RST = 1  */
    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_RST, Bit_RESET );  /* RST = 0  */
//    starting_delay( 0x500 );
    Delayms( 50 );

    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_RST, Bit_SET );    /* RST = 1  */
//    starting_delay( 0x500 );
    Delayms( 50 );

    /*default mode is output*/
    LCD_DataLinesConfig( Output );

    LCD_CheckLCDStatus();

    LCD_SendLCDCmd( ST7637_SWRESET );

    /*-----------disable autoread + Manual read once ----------------------------*/
    LCD_SendLCDCmd( ST7637_AUTOLOADSET );  /* Auto Load Set 0xD7*/
    LCD_SendLCDData( 0xBF );               /* Auto Load Disable*/

    LCD_SendLCDCmd( ST7637_EPCTIN );       /* EE Read/write mode 0xE0*/
    LCD_SendLCDData( 0x00 );               /* Set read mode*/

    LCD_SendLCDCmd( ST7637_EPMRD );        /* Read active 0xE3*/
    LCD_SendLCDCmd( ST7637_EPCTOUT );      /* Cancel control 0xE1*/

    /*---------------------------------- Sleep OUT ------------------------------*/
    LCD_SendLCDCmd( ST7637_DISPOFF );      /* display off 0x28*/
    LCD_SendLCDCmd( ST7637_SLPOUT );       /* Sleep Out 0x11*/

    /*--------------------------------Vop setting--------------------------------*/
    LCD_SendLCDCmd( ST7637_VOPSET );       /* Set Vop by initial Module 0xC0*/
    LCD_SendLCDData( 0xFB );               /* Vop = 13.64*/
    LCD_SendLCDData( 0x00 );               /* base on Module*/

    /*----------------------------Set Register-----------------------------------*/
    LCD_SendLCDCmd( ST7637_BIASSEL );      /* Bias select 0xC3*/
    LCD_SendLCDData( 0x00 );               /* 1/12 Bias, base on Module*/

    LCD_SendLCDCmd( ST7637_BSTBMPXSEL );   /* Setting Booster times 0xC4*/
    LCD_SendLCDData( 0x05 );               /* Booster X 8*/

    LCD_SendLCDCmd( ST7637_BSTEFFSEL );    /* Booster eff 0xC5*/
    LCD_SendLCDData( 0x11 );               /* BE = 0x01 (Level 2)*/

    LCD_SendLCDCmd( ST7637_VGSORCSEL );    /* Vg with booster x2 control 0xcb*/
    LCD_SendLCDData( 0x01 );               /* Vg from Vdd2*/

    LCD_SendLCDCmd( ST7637_ID1SET );       /* ID1 = 00 0xcc*/
    LCD_SendLCDData( 0x00 );               /**/

    LCD_SendLCDCmd( ST7637_ID3SET );       /* ID3 = 00 0xce*/
    LCD_SendLCDData( 0x00 );               /**/

    LCD_SendLCDCmd( 0xB7 );                /* Glass direction*/
    LCD_SendLCDData( 0xC0 );               /**/

    LCD_SendLCDCmd( ST7637_ANASET );       /* Analog circuit setting 0xd0*/
    LCD_SendLCDData( 0x1D );               /**/

    LCD_SendLCDCmd( 0xB4 );                /* PTL mode set*/
    LCD_SendLCDData( 0x18 );               /* power normal mode*/
    LCD_SendLCDCmd( ST7637_INVOFF );       /* Display Inversion OFF 0x20*/

    LCD_SendLCDCmd( 0x2A );                /* column range*/
    LCD_SendLCDData( 0x04 );               /**/
    LCD_SendLCDData( 0x83 );               /**/

    LCD_SendLCDCmd( 0x2B );                /* raw range*/
    LCD_SendLCDData( 0x04 );               /**/
    LCD_SendLCDData( 0x83 );               /**/


    LCD_SendLCDCmd( ST7637_COLMOD );       /* Color mode = 65k 0x3A*/
    LCD_SendLCDData( 0x05 );               /**/

    LCD_SendLCDCmd( ST7637_MADCTR );       /* Memory Access Control 0x36*/
    LCD_SendLCDData( V9_MADCTRVAL );

    LCD_SendLCDCmd( ST7637_DUTYSET );      /* Duty = 132 duty 0xb0*/
    LCD_SendLCDData( 0x7F );

    LCD_SendLCDCmd( 0x29 );                /* Display ON*/
    LCD_SendLCDCmd( 0xF9 );                /* Gamma*/
    LCD_SendLCDData( 0x00 );               /**/
    LCD_SendLCDData( 0x03 );               /**/
    LCD_SendLCDData( 0x05 );               /**/
    LCD_SendLCDData( 0x07 );               /**/
    LCD_SendLCDData( 0x09 );               /**/
    LCD_SendLCDData( 0x0B );               /**/
    LCD_SendLCDData( 0x0D );               /**/
    LCD_SendLCDData( 0x0F );               /**/
    LCD_SendLCDData( 0x11 );               /**/
    LCD_SendLCDData( 0x13 );               /**/
    LCD_SendLCDData( 0x15 );               /**/
    LCD_SendLCDData( 0x17 );               /**/
    LCD_SendLCDData( 0x19 );               /**/
    LCD_SendLCDData( 0x1B );               /**/
    LCD_SendLCDData( 0x1D );               /**/
    LCD_SendLCDData( 0x1F );               /**/
}
#endif /*PRIMER1*/


#ifdef PRIMER2
/*******************************************************************************
*
*                                LCD_ST7732S_Controller_init
*
*******************************************************************************/
/**
*  Initialization of the controller registers.
*
**/
/******************************************************************************/
NODEBUG void LCD_ST7732S_Controller_init( void )
{

#define WriteCOM LCD_SendLCDCmd
#define WriteData LCD_SendLCDData

#define WRITE_LCD(addr,val)   { WriteCOM(val); }

    Delayms( 100 );
    GPIO_WriteBit( GPIOD, CtrlPin_RST, Bit_RESET ); /* RST = 0  */
    Delayms( 100 );
    GPIO_WriteBit( GPIOD, CtrlPin_RST, Bit_SET );   /* RST = 1  */
    Delayms( 100 );

    WriteCOM( ST7732_SWRESET );        /*Software Reset*/
    Delayms( 180 );                    /*DELAY 150MS*/

    WriteCOM( ST7732_SLPIN );                  /*sleep IN*/
    Delayms( 100 );
    WriteCOM( ST7732_SLPOUT );                  /*sleep out*/
    Delayms( 150 );

    /* WriteCOM(0xc5);                   //VCOM Control 1*/
    /* WriteData(40);                    //set vcomh=4.475V*/
    /* WriteData(45);                    //SET VCOML=-0.800V*/

    WriteCOM( ST7732_FRMCTR1 );        /*    frame*/
    WriteData( 0x06 );
    WriteData( 0x03 );
    WriteData( 0x02 );
    //Delayms(10);

    WriteCOM( ST7732_INVCTR );         /*   0xb4*/
    WriteData( 0x03 );

    WriteCOM( ST7732_DISSET5 );        /*  frame 0xB6*/
    WriteData( 0x02 );
    WriteData( 0x0e );

    WriteCOM( ST7732_DISPCTRL );       /* 0xF5*/
    WriteData( 0x1a );

    WriteCOM( 0xc0 );                  /*Power Control 1 ( gvdd vci1)*/
    WriteData( 0x02 );
    WriteData( 0x00 );

    WriteCOM( 0xc1 );                  /*Power Control 2( avdd vcl vgh vgl) */
    WriteData( 0x05 );

    WriteCOM( 0xc2 );                  /*Power Control 3 ( in normal)   */
    WriteData( 0x02 );
    WriteData( 0x02 );

    WriteCOM( 0xc3 );                  /*Power Control 4 ( in Idle mode)   */
    WriteData( 0x01 );
    WriteData( 0x02 );

    WriteCOM( 0xc4 );                  /*Power Control 5 ( in partial mode)  */
    WriteData( 0x01 );
    WriteData( 0x02 );

    WriteCOM( 0xc5 );                  /*Vcom Control ( vcomh,voml)  */
    WriteData( 0x47 );
    WriteData( 0x3a );

    WriteCOM( ST7732_OSCADJ );         /* 0xF2 intern OSC 80Hz*/
    WriteData( 0x02 );

    WriteCOM( ST7732_DEFADJ );         /*0xF6*/
    WriteData( 0x4c );

    WriteCOM( 0xf8 ); /*cmd 0xf8,dat 0x06; LOAD */
    WriteData( 0x06 ); /*SLEEP OUT LOAD DEFAULT*/


    /****************gamma adjust **********************/
    WriteCOM( 0xe0 );                  /*gamma*/
    WriteData( 0x06 );
    WriteData( 0x1c );
    WriteData( 0x1f );
    WriteData( 0x1f );
    WriteData( 0x18 );
    WriteData( 0x13 );
    WriteData( 0x06 );
    WriteData( 0x03 );
    WriteData( 0x03 );
    WriteData( 0x04 );
    WriteData( 0x07 );
    WriteData( 0x07 );
    WriteData( 0x00 );

    WriteCOM( 0xe1 );                  /*gamma*/
    WriteData( 0x0a );
    WriteData( 0x14 );
    WriteData( 0x1b );
    WriteData( 0x18 );
    WriteData( 0x12 );
    WriteData( 0x0e );
    WriteData( 0x02 );
    WriteData( 0x01 );
    WriteData( 0x00 );
    WriteData( 0x01 );
    WriteData( 0x08 );
    WriteData( 0x07 );
    WriteData( 0x00 );

    WriteCOM( ST7732_MADCTR );         /* Memory Access Control 0x36  */
    WriteData( V12_MADCTRVAL );

    WriteCOM( 0x2a );                  /*Column Range*/
    WriteData( 0x00 );
    WriteData( 0x00 );
    WriteData( 0x00 );
    WriteData( 0x7F );

    WriteCOM( 0x2b );                  /*page Range*/
    WriteData( 0x00 );
    WriteData( 0x00 );
    WriteData( 0x00 );
    WriteData( 0x7F );                 /* configured as 128x128*/

    WriteCOM( ST7732_COLMOD );         /* Color mode = 65k 0x3A*/
    WriteData( 0x55 );

    WriteCOM( ST7732_TEON );           /* 0x35*/
    WriteData( 0x00 );

    WriteCOM( 0x29 );                   /*display on*/
    Delayms( 20 );


    /*LCD_FillRect ( 0,0,161,131, RGB_WHITE );*/
    LCD_FillRect_Circle( 0, 0, 131, 161, RGB_WHITE );

}

#endif /*PRIMER2*/

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
NODEBUG void LCD_BackLightConfig( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable GPIO clock  */
    RCC_APB2PeriphClockCmd( GPIO_LCD_BL_PERIPH, ENABLE );

    /* GPIO Configuration : TIM in Output */
    GPIO_InitStructure.GPIO_Pin   = GPIO_BACKLIGHT_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOx_BL_LCD, &GPIO_InitStructure );

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
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
    TIM_OCxInit( TIM_LCD_BL, &TIM_OCInitStructure );

    TIM_OCxPreloadConfig( TIM_LCD_BL, TIM_OCPreload_Disable );

    TIM_ARRPreloadConfig( TIM_LCD_BL, ENABLE );

    /* Go !!!*/
    TIM_Cmd( TIM_LCD_BL, ENABLE );
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
NODEBUG void LCD_BackLightChange( void )
{
    /* Output Compare Toggle Mode configuration: Channelx */
    TIM_OCInitStructure.TIM_Pulse = Current_CCR_BackLightStart;

    TIM_OCxInit( TIM_LCD_BL, &TIM_OCInitStructure );
}

/* Public functions for CircleOS ---------------------------------------------*/

/*******************************************************************************
*
*                                LCD_Init
*
*******************************************************************************/
/**
*
*  Initialize LCD. Called at CircleOS startup.
*
*  @attention  This function must <b>NOT</b> be called by the user.
*
**/
/******************************************************************************/
NODEBUG void LCD_Init( void )
{
    Screen_Width = APP_SCREEN_WIDTH;
    Screen_Height = APP_SCREEN_HEIGHT;

    // Starting delay (for LCD startup)
    Delayms( 10 );

    LCD_SetBackLight( UTIL_ReadBackupRegister( BKP_BKLIGHT ) );

    /* Enable GPIO clock for LCD */
    RCC_APB2PeriphClockCmd( GPIO_LCD_CTRL_PERIPH, ENABLE );
    RCC_APB2PeriphClockCmd( GPIO_LCD_D_PERIPH, ENABLE );
    RCC_APB2PeriphClockCmd( GPIO_LCD_CS_PERIPH, ENABLE );

    /* Enable GPIOC clock */
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE );

    /* Init BackLight*/
    LCD_BackLightConfig();

    /* LCD Init */
#ifdef PRIMER1
    /* Configure control lines signals as output mode */
    LCD_CtrlLinesConfig();
    /* 7637 init */
    LCD_7637_Controller();
#endif /* PRIMER1*/

#ifdef PRIMER2

    /* Enable the FSMC Clock */
    RCC_AHBPeriphClockCmd( RCC_AHBPeriph_FSMC, ENABLE );

    /* Configure FSMC Bank1 NOR/SRAM3 */
    LCD_FSMC_Init();

    LCD_ST7732S_Controller_init();
#endif /*PRIMER2*/

    LCD_SetOffset( OFFSET_OFF );
    LCD_SetDefaultFont();
    LCD_SetTransparency(0);
}

#ifdef PRIMER2
/*******************************************************************************
*
*                                LCD_FSMC_Init
*
*******************************************************************************/
/**
*
*  @attention  This function must <b>NOT</b> be called by the user.
*
**/
/******************************************************************************/
NODEBUG void LCD_FSMC_Init( void )
{
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef  p;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOD | GPIO_LCD_D_PERIPH, ENABLE );


    /*-- GPIO Configuration ------------------------------------------------------*/
    /* SRAM Data lines configuration */

    GPIO_InitStructure.GPIO_Pin = LCD_DATA_PINS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOx_D_LCD, &GPIO_InitStructure );

    /* NOE, NWE and NE1 configuration */
    GPIO_InitStructure.GPIO_Pin = CtrlPin_RD | CtrlPin_WR | CtrlPin_RS;
    GPIO_Init( GPIOx_CTRL_LCD, &GPIO_InitStructure );

    /* Reset : configured as regular GPIO, is not FSMC-controlled */
    GPIO_InitStructure.GPIO_Pin = CtrlPin_RST | CtrlPin_CS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init( GPIOD, &GPIO_InitStructure );

    GPIO_WriteBit( GPIOD, CtrlPin_RST, Bit_SET ); /*reset active at LOW*/
    GPIO_WriteBit( GPIOD, CtrlPin_CS, Bit_RESET ); /*reset active at LOW*/


    /*-- FSMC Configuration ------------------------------------------------------*/
    p.FSMC_AddressSetupTime = 2;
    p.FSMC_AddressHoldTime = 2;
    p.FSMC_DataSetupTime = 2;
    p.FSMC_BusTurnAroundDuration = 5;
    p.FSMC_CLKDivision = 5;
    p.FSMC_DataLatency = 5;
    p.FSMC_AccessMode = FSMC_AccessMode_A;

    FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM3;
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
    FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
    FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;
    FSMC_NORSRAMInit( &FSMC_NORSRAMInitStructure );

    /* Enable FSMC Bank1_SRAM Bank */
    FSMC_NORSRAMCmd( FSMC_Bank1_NORSRAM3, ENABLE );
}
#endif /*PRIMER2*/



/*******************************************************************************
*
*                                LCD_SendLCDCmd_RAM_Access
*
*******************************************************************************/
/**
*   Call LCD_SendLCDCmd function, with the ST7637_RAMWR or ST7732_RAMWR parameter
*
*
**/
/******************************************************************************/
NODEBUG void LCD_SendLCDCmd_RAM_Access( void )
{

#ifdef PRIMER1
    /* Send command to write data on the LCD screen.*/
    LCD_SendLCDCmd( ST7637_RAMWR );
#endif

#ifdef PRIMER2
    LCD_SendLCDCmd( ST7732_RAMWR );
#endif

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
NODEBUG void LCD_SendLCDCmd_RAM_Access_End( void )
{
}

/*******************************************************************************
*
*                                LCD_SendLCDCmd_Rotation_Access
*
*******************************************************************************/
/**
*   Call LCD_SendLCDCmd function, with the ST7637_MADCTR or ST7732_MADCTR parameter
*
*
**/
/******************************************************************************/
NODEBUG void LCD_SendLCDCmd_Rotation_Access( void )
{

#ifdef PRIMER1
    /* Memory Access Control 0x36*/
    LCD_SendLCDCmd( ST7637_MADCTR );
#endif /*PRIMER1*/

#ifdef PRIMER2
    /* Memory Access Control 0x36*/
    LCD_SendLCDCmd( ST7732_MADCTR );
#endif /*PRIMER2*/

}

/*******************************************************************************
*
*                                LCD_Batt
*
*******************************************************************************/
/**
*   Draw the battery
*   Depends on the Primer version
*
**/
/******************************************************************************/
NODEBUG void LCD_Batt( coord_t xBat, coord_t yBat, bool fDisplayTime, u16 BatState, u16* OldBatState, divider_t divider_coord, coord_t* widthBat, coord_t* heightBat )
{
#ifdef PRIMER1
    if ((( divider_coord++ % 500 ) == 0 ) &&
            (( BatState < ( *OldBatState - 100 ) ) || ( BatState > ( *OldBatState + 100 ) ) ) &&
            ( fDisplayTime == 1 ) && ( BatState < NO_BAT_VOLTAGE ) )
    {
        *OldBatState = BatState;
        *heightBat   = 4;
//        *widthBat    = ( 2 * ( BatState - 2000 ) ) / 70 ; YRT20100126 bug divider bridge
        *widthBat    = (( BatState - 2000 ) ) / 70 ;

        if ( *widthBat > 20 )
        {
            *widthBat = 20;
        }

        LCD_FillRect_Circle(( xBat + 22 ) - *widthBat , yBat + 2 , *widthBat , *heightBat , RGB_BLUE );
        LCD_FillRect_Circle( xBat + 2 , yBat + 2 , 20 - *widthBat , *heightBat , RGB_LIGHTBLUE );
    }
#endif

#if PRIMER2
    static u32  last_time = 0;
    u32 new_time = RTC_GetCounter();

    if (( fDisplayTime == 1 ) && ( last_time != new_time ) )
    {
        s32 bat_color;
        s32 fill;
        bool fBlink = FALSE;

        last_time = new_time;
        switch ( PWR_CurrentState )
        {
        case PWR_STATE_UNDEF: return;
        case PWR_STATE_NOBAT:
            bat_color = RGB_PINK;
            fill = 100;
            fBlink = TRUE;
            break;

        case PWR_STATE_CHARGING:
            bat_color = RGB_BLUE;
            fill = 50;
            fBlink = TRUE;
            break;

        case PWR_STATE_FULL:
            bat_color = RGB_GREEN;
            fill = 100;
            fBlink = FALSE;
            break;

        case PWR_STATE_NORMAL:
            bat_color = RGB_BLACK;
            fBlink = FALSE;
            fill = PWR_BatteryLevel;
            break;

        case PWR_STATE_LOW:
            bat_color = RGB_YELLOW;
            fBlink = TRUE;
            fill = PWR_BatteryLevel;
            break;

        case PWR_STATE_EMPTY:
            bat_color = RGB_RED;
            fill = 100;
            fBlink = TRUE;
            break;
        }

        *widthBat = ( 20 * fill ) / 100;
        if ( *widthBat > 20 )
        {
            *widthBat = 20;
        }
        if ( fBlink && ( new_time & 1 ) )
        {
            LCD_FillRect_Circle(( xBat + 22 ) - *widthBat , yBat + 2 , *widthBat , *heightBat , ( PWR_CurrentState == PWR_STATE_LOW ) ? bat_color : RGB_WHITE );
            LCD_FillRect_Circle( xBat + 2 , yBat + 2 , 20 - *widthBat , *heightBat , ( PWR_CurrentState == PWR_STATE_LOW ) ? bat_color : RGB_WHITE );
        }
        else
        {
            LCD_FillRect_Circle(( xBat + 22 ) - *widthBat , yBat + 2 , *widthBat , *heightBat , ( PWR_CurrentState == PWR_STATE_LOW ) ? RGB_WHITE : bat_color );
            LCD_FillRect_Circle( xBat + 2 , yBat + 2 , 20 - *widthBat , *heightBat , ( PWR_CurrentState == PWR_STATE_LOW ) ? RGB_RED : RGB_WHITE );
        }
    }
#endif

}

/*******************************************************************************
*
*                                LCD_Clear
*
*******************************************************************************/
/**
*   Draw the batterie and the toolbar
*   Depends on the Primer version
*
**/
/******************************************************************************/
NODEBUG void LCD_Clear( u16 xBat, u16  yBat )
{

#ifdef PRIMER2
    if ( TOUCHSCR_GetMode() != TS_CALIBRATION )
    {
        TOUCHSCR_SetMode( TS_DRAWING );
    }
#endif

#ifdef PRIMER1
    /* Draw Image*/
    if ( UTIL_GetBat() < NO_BAT_VOLTAGE )
    {
        DRAW_Batt();
    }
    else
    {
        LCD_FillRect_Circle( xBat - 2 , yBat , 26 , 8 , RGB_WHITE );
        DRAW_SetTextColor( RGB_BLACK );
        DRAW_DisplayString( xBat - 22, yBat - 3, "NO BATT", 7 );
    }
#endif

    // DRAW_SetLogoBW();

#ifdef PRIMER2
    DRAW_Batt();

    TOOLBAR_SetDefaultToolbar();
#endif

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
NODEBUG void LCD_SetLogoBW( void )
{
#ifdef PRIMER2
    const u16 primer2pic[] =
    {
#   include "bmp\\primer2pic_RLE256.h"
    };
    DRAW_SetImage( primer2pic, 0, 0, 128, 128 ) ; /* 128x128 Image Size*/
#else
    /* Save current text color.*/
    color_t l_u16TextColor   = TextColor;
    s32 i;

    TextColor = RGB_BLACK;

    DRAW_SetImageBW( butterfly_bw, 10 + 5 + BUTTERFLYBABY_WIDTH / 2 + ( Screen_Width - BUTTERFLY_HEIGHT ) / 2, ( Screen_Height - BUTTERFLY_WIDTH ) / 2, BUTTERFLY_WIDTH, BUTTERFLY_HEIGHT );
    DRAW_SetImageBW( butterflybaby_bw, 10 + 2 + BUTTERFLYBABY_WIDTH, ( Screen_Height - BUTTERFLYBABY_WIDTH ) / 2 - 10, BUTTERFLYBABY_WIDTH, BUTTERFLYBABY_HEIGHT );
    DRAW_SetImageBW( butterflybaby_bw, 10, 10 + ( Screen_Height - BUTTERFLYBABY_WIDTH ) / 2, BUTTERFLYBABY_WIDTH, BUTTERFLYBABY_HEIGHT );

    TextColor = RGB_LIGHTBLUE;
    DRAW_SetImageBW( st_bw, 2, ( Screen_Height - ST_HEIGHT ) - 5, ST_WIDTH, ST_HEIGHT );

    TextColor = RGB_RED;
    DRAW_SetImageBW( raiso_bw, Screen_Width - RAISO_WIDTH - 5, ( Screen_Height - RAISO_HEIGHT - 10 ), RAISO_WIDTH, RAISO_HEIGHT );


    DRAW_SetTextColor( RGB_BLUE );
    DRAW_DisplayString( 22, 26, "STM32 Primer", 12 );
#ifdef PRIMER2
    DRAW_DisplayString( 108, 26, "2", 12 );
#endif

    /* Restore text color.*/
    TextColor = l_u16TextColor;
#endif
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
NODEBUG void LCD_Scroll( u8 Ascii, coord_t PosCurX, coord_t* PosCurY,
                         coord_t RightMarginX, coord_t LeftMarginX, coord_t HighMarginY, coord_t LowMarginY,
                         color_t BGndColor, color_t TextColor, mag_t CharMagniCoeff )
{
#ifdef PRIMER2

    s32 Nblines, width, i, y;

    InitListDMA();
    Nblines = ( LowMarginY - *PosCurY ) / DELTA_Y;
    width = RightMarginX - LeftMarginX;
    for ( i = 0; i < Nblines ; i++ )
    {

        for ( y = HighMarginY - DELTA_Y; y >= LowMarginY; y -= DELTA_Y )
        {
            if ( width < PHYS_SCREEN_WIDTH )
                {
                LIST_LCD_RectRead( LeftMarginX, y - DELTA_Y, width, DELTA_Y );
                LIST_DRAW_SetImage( LeftMarginX, y, width, DELTA_Y );
                }
            else
                {
                LIST_LCD_RectRead( LeftMarginX, y - DELTA_Y, PHYS_SCREEN_WIDTH, DELTA_Y );
                LIST_DRAW_SetImage( LeftMarginX, y, PHYS_SCREEN_WIDTH, DELTA_Y );
                LIST_LCD_RectRead( LeftMarginX + PHYS_SCREEN_WIDTH, y - DELTA_Y, width - PHYS_SCREEN_WIDTH, DELTA_Y );
                LIST_DRAW_SetImage( LeftMarginX + PHYS_SCREEN_WIDTH, y, width - PHYS_SCREEN_WIDTH, DELTA_Y );
                }
        }
        LCD_FillRect( LeftMarginX, LowMarginY, width, DELTA_Y, BGndColor );
    }
    *PosCurY = LowMarginY;

    // Display the selected bitmap according to the provided ASCII character.
    LCD_DisplayChar( PosCurX, *PosCurY, Ascii, TextColor, BGndColor, CharMagniCoeff );

#endif
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
NODEBUG void LCD_DrawPixel( coord_t XPos, coord_t YPos, color_t Color )
{
    /* Select LCD screen area. */
    LCD_SetRect_For_Cmd( XPos, YPos, 1, 1 );

#ifdef PRIMER1
    /* Send LCD RAM write command. */
    LCD_SendLCDCmd( ST7637_RAMWR );
#endif /*PRIMER1*/

#ifdef PRIMER2
    /* Send LCD RAM write command. */
    LCD_SendLCDCmd( ST7732_RAMWR );
#endif /*PRIMER2*/

    /* Draw pixel.*/
    LCD_SendLCDData( Color );
    LCD_SendLCDData( Color >> 8 );
}

/*******************************************************************************
*
*                                LCD_RectRead
*
*******************************************************************************/
/**
*
*  Save the pixels of a rectangle part of the LCD into a RAM variable.
*
*  @param[in]  x        The horizontal coordinate of the rectangle low left corner.
*  @param[in]  y        The vertical coordinate of the rectangle low left corner.
*  @param[in]  width    The rectangle width in pixels.
*  @param[in]  height   The rectangle height in pixels.
*  @param[out] bmp      The variable to store the read data into.
*
*  @warning    One pixel weights 2 bytes.
*  @warning    The (0x0) point in on the low left corner.
*
**/
/******************************************************************************/
NODEBUG void LCD_RectRead( coord_t x, coord_t y, coord_t width, coord_t height, u8* bmp )
{
    s32 i;
    s32 bytesize = ( width * height ) * 2; /* 2 bytes per pixel.*/

    /* Select LCD screen area. */
    LCD_SetRect_For_Cmd( x, y, width, height );

#ifdef PRIMER1
    /* Send LCD RAM read command. */
    LCD_SendLCDCmd( ST7637_RAMRD );
#endif /*PRIMER1*/

#ifdef PRIMER2
    /* Restore 18 bit color mode */
    LCD_SendLCDCmd( ST7732_COLMOD );
    LCD_SendLCDData( 0x66 );

    /* Send LCD RAM read command. */
    LCD_SendLCDCmd( ST7732_RAMRD );
#endif /*PRIMER2*/

    /* First read byte is dummy!*/
    LCD_ReadLCDData();

    /* Read pixels from LCD screen.*/
    for ( i = 0; i < bytesize; i++ )
    {
#ifdef PRIMER2
        u16 red_val  =   LCD_ReadLCDData() ;
        u16 byte0 = red_val & 0xf8;   /*red: keep only 5 bits */
        u16 green_val = LCD_ReadLCDData();
        *bmp++ = ( byte0 | (( green_val >> 5 ) & 0x7 ) );
        u16 blue_val =  LCD_ReadLCDData();
        *bmp++ = ((( blue_val >> 3 ) & 0x1F ) | (( green_val << 3 ) & 0xE0 ) );
        i++; /*don't multiply by two...*/
#else
        *bmp++ = LCD_ReadLCDData();
#endif
    }
#ifdef PRIMER2
    /* Restore 16 bit color mode */
    LCD_SendLCDCmd( ST7732_COLMOD );
    LCD_SendLCDData( 0x55 );
#endif
}

/*******************************************************************************
*
*                                LCD_GetPixel
*
*******************************************************************************/
/**
*
*  Read the RGB color of the pixel the coordinate are provided in parameter.
*
*  @param[in]  x        The horizontal coordinate of the pixel.
*  @param[in]  y        The vertical coordinate of the pixel.
*  @return              An unsigned 16 bit word containing the RGB color of the pixel.
*
*  @warning    The (0x0) point in on the low left corner.
*  @see        LCD_RectRead
*
**/
/******************************************************************************/
NODEBUG color_t LCD_GetPixel( coord_t x, coord_t y )
{
    color_t val;

    LCD_RectRead( x, y, 1, 1, ( u8* )&val );

    return val;
}


/*******************************************************************************
*
*                                LCD_DisplayRotate
*
*******************************************************************************/
/**
*  Configure the LCD controller for a given orientation.
*
*  @param[in]  H12 The new screen orientation.
*
**/
/******************************************************************************/
NODEBUG void LCD_DisplayRotate( Rotate_H12_V_Match_TypeDef H12 )
{

    /* Memory Access Control */
    LCD_SendLCDCmd_Rotation_Access();

    // Apply desired direction
    switch ( H12 )
    {
    case V3  :
        LCD_SendLCDData( V3_MADCTRVAL & 0xFF );
        break;

    case V6  :
        LCD_SendLCDData( V6_MADCTRVAL & 0xFF );
        break;

    case V9  :
        LCD_SendLCDData( V9_MADCTRVAL & 0xFF );
        break;

    case V12 :
    default  :
        LCD_SendLCDData( V12_MADCTRVAL & 0xFF );
        break;

    case V3BMP  :
        LCD_SendLCDData( V3BMP_MADCTRVAL & 0xFF );
        break;

    case V6BMP  :
        LCD_SendLCDData( V6BMP_MADCTRVAL & 0xFF );
        break;

    case V9BMP  :
        LCD_SendLCDData( V9BMP_MADCTRVAL & 0xFF );
        break;

    case V12BMP :
        LCD_SendLCDData( V12BMP_MADCTRVAL & 0xFF );
        break;
    }
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
NODEBUG void LCD_SendLCDCmd( u8 Cmd )
{
#ifdef PRIMER1
    /* Start the LCD send data sequence */
    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_RS, Bit_RESET );     /* RS = 0 */
    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_RD, Bit_SET );       /* RD = 1 */
    LCD_CtrlLinesWrite( GPIOx_CS_LCD,   CtrlPin_CS, Bit_RESET );     /* CS = 0 */
    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_WR, Bit_RESET );     /* WR = 0 */

    /* Write data to the LCD */
    LCD_DataLinesWrite( GPIOx_D_LCD, ( u32 )Cmd );
    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_WR, Bit_SET );       /* WR = 1 */
#endif /*PRIMER1*/

#ifdef PRIMER2
    /* Send command to the LCD */
    *( u16 volatile* )( LCD_CMD_MODE_ADDR ) = ( Cmd << 4 );
#endif /*PRIMER2*/
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
NODEBUG void LCD_SendLCDData( u8 Data )
{
#ifdef PRIMER1
    /* Configure Data lines as Output */
    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_RS, Bit_SET );
    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_RD, Bit_SET );
    LCD_CtrlLinesWrite( GPIOx_CS_LCD,   CtrlPin_CS, Bit_RESET );
    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_WR, Bit_RESET );

    /* Write data to the LCD */
    LCD_DataLinesWrite( GPIOx_D_LCD, ( u32 )Data );
    LCD_CtrlLinesWrite( GPIOx_CTRL_LCD, CtrlPin_WR, Bit_SET );
#endif /*PRIMER1   */

#ifdef PRIMER2
    /* Transfer data to the FSMC */
    *( u16 volatile* )( LCD_DATA_MODE_ADDR ) = ( Data << 4 );
#endif /*PRIMER2*/

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
lcdt_t LCD_ReadLCDData( void )
{
#ifdef PRIMER1
    u32 LCDData = 0;

    /* Configure Data lines as Input */
    LCD_DataLinesConfig( Input );

    /* Start the LCD send data sequence */
    LCD_CtrlLinesWrite( GPIOx_D_LCD,  CtrlPin_RS, Bit_SET );         /* RS = 1 */
    LCD_CtrlLinesWrite( GPIOx_D_LCD,  CtrlPin_WR, Bit_SET );         /* WR = 1 */
    LCD_CtrlLinesWrite( GPIOx_CS_LCD, CtrlPin_CS, Bit_RESET );       /* CS = 0 */
    LCD_CtrlLinesWrite( GPIOx_D_LCD,  CtrlPin_RD, Bit_RESET );       /* RD = 0 */

    /* Read data from the LCD */
    LCDData = ( GPIO_ReadInputData( GPIOx_D_LCD ) & LCD_DATA_PINS );

    LCD_CtrlLinesWrite( GPIOx_D_LCD, CtrlPin_RD, Bit_SET );          /* RD = 1 */

    /* Read the LCD returned data */
    LCD_DataLinesConfig( Output );

    return LCDData;
#endif /*PRIMER1*/

#ifdef PRIMER2
    /* Transfer data from the memory */
    return (( *( u16 volatile* )( LCD_DATA_MODE_ADDR ) )  >> 4 ) ;
#endif /*PRIMER2*/
}


/*******************************************************************************
*
*                                LCD_SetRect_For_Cmd
*
*******************************************************************************/
/**
*
*  Define the rectangle for the next command to be applied.
*
*  @param[in]  x        The horizontal coordinate of the rectangle low left corner.
*  @param[in]  y        The vertical coordinate of the rectangle low left corner.
*  @param[in]  width    The rectangle width in pixels.
*  @param[in]  height   The rectangle height in pixels.
*
*  @warning    The (0x0) point in on the low left corner.
*
**/
/******************************************************************************/
void LCD_SetRect_For_Cmd( coord_t x, coord_t y, coord_t width, coord_t height )
{
#ifdef PRIMER1
if ( CurrentScreenOrientation <4 )
{
    LCD_SendLCDCmd( ST7637_CASET );
    LCD_SendLCDData( y + OrientationOffsetY[ CurrentScreenOrientation ] );
    LCD_SendLCDData( y + OrientationOffsetY[ CurrentScreenOrientation ] + height - 1 );

    LCD_SendLCDCmd( ST7637_RASET );
    LCD_SendLCDData( x + OrientationOffsetX[ CurrentScreenOrientation ] );
    LCD_SendLCDData( x + OrientationOffsetX[ CurrentScreenOrientation ] + width - 1 );
}
else
{
    LCD_SendLCDCmd( ST7637_CASET );
    LCD_SendLCDData( x + OrientationOffsetX[ CurrentScreenOrientation ] );
    LCD_SendLCDData( x + OrientationOffsetX[ CurrentScreenOrientation ] + width - 1 );

    LCD_SendLCDCmd( ST7637_RASET );
    LCD_SendLCDData( y + OrientationOffsetY[ CurrentScreenOrientation ] );
    LCD_SendLCDData( y + OrientationOffsetY[ CurrentScreenOrientation ] + height - 1 );
}
#endif /*PRIMER1*/

#ifdef PRIMER2
if ( CurrentScreenOrientation <4 )
{
    LCD_SendLCDCmd( ST7732_CASET );
    LCD_SendLCDData( 0 );
    LCD_SendLCDData( y + OrientationOffsetY[ CurrentScreenOrientation ] );
    LCD_SendLCDData( 0 );
    LCD_SendLCDData( y + OrientationOffsetY[ CurrentScreenOrientation ] + height - 1 );
    LCD_SendLCDCmd( ST7732_RASET );
    LCD_SendLCDData( 0 );
    LCD_SendLCDData( x + OrientationOffsetX[ CurrentScreenOrientation ] );
    LCD_SendLCDData( 0 );
    LCD_SendLCDData( x + OrientationOffsetX[ CurrentScreenOrientation ] + width - 1 );
}
else
{
    LCD_SendLCDCmd( ST7732_CASET );
    LCD_SendLCDData( 0 );
    LCD_SendLCDData( x + OrientationOffsetX[ CurrentScreenOrientation ] );
    LCD_SendLCDData( 0 );
    LCD_SendLCDData( x + OrientationOffsetX[ CurrentScreenOrientation ] + width - 1 );
    LCD_SendLCDCmd( ST7732_RASET );
    LCD_SendLCDData( 0 );
    LCD_SendLCDData( y + OrientationOffsetY[ CurrentScreenOrientation ] );
    LCD_SendLCDData( 0 );
    LCD_SendLCDData( y + OrientationOffsetY[ CurrentScreenOrientation ] + height - 1 );
}
#endif /*PRIMER2*/
}

/*******************************************************************************
*
*                                LCD_FullScreen
*
*******************************************************************************/
/**
*
*  Enable or disable the possibility to use all the screen.
*
*   @param[in] FullScreenONOFF : set the full screen mode : 1=ON, 0=OFF
*
**/
/******************************************************************************/
void LCD_FullScreen( u8 FullScreenONOFF )
{
    extern coord_t CurrentPointerWidth;
    extern coord_t CurrentPointerHeight;

    if ( FullScreenONOFF != 0 )
        {
#if TOUCHSCREEN_AVAIL
        UTIL_SetSchHandler(TOOLBAR_SCHHDL_ID,0);
#endif
        LCD_SetRotateScreen(0);
        LCD_SetOffset(OFFSET_OFF);
#ifdef PRIMER2
        OrientationOffsetX[ CurrentScreenOrientation ] = 0;
        OrientationOffsetY[ CurrentScreenOrientation ] = 0;
        OrientationOffsetX[ CurrentScreenOrientation + 4 ] = 0;
        OrientationOffsetY[ CurrentScreenOrientation + 4 ] = 0;
#endif
        Screen_Width = PHYS_SCREEN_WIDTH;
        Screen_Height = PHYS_SCREEN_HEIGHT;
        if( (LCD_GetScreenOrientation()%2) == 1 )
            {
            PosCurY = PHYS_SCREEN_WIDTH - Char_Height;
            RightMarginX = PHYS_SCREEN_HEIGHT;
            HighMarginY = PHYS_SCREEN_WIDTH;
            LCD_DrawCharSetFilter( 0, PHYS_SCREEN_HEIGHT, 0, PHYS_SCREEN_WIDTH );
            POINTER_SetRect( 0, 0, PHYS_SCREEN_HEIGHT - CurrentPointerWidth, PHYS_SCREEN_WIDTH - CurrentPointerHeight);
            }
        else
            {
            PosCurY = PHYS_SCREEN_HEIGHT - Char_Height;
            RightMarginX = PHYS_SCREEN_WIDTH;
            HighMarginY = PHYS_SCREEN_HEIGHT;
            LCD_DrawCharSetFilter( 0, PHYS_SCREEN_WIDTH, 0, PHYS_SCREEN_HEIGHT );
            POINTER_SetRect( 0, 0, PHYS_SCREEN_WIDTH - CurrentPointerWidth, PHYS_SCREEN_HEIGHT - CurrentPointerHeight);
            }
        }
    else
        {
#if TOUCHSCREEN_AVAIL
        UTIL_SetSchHandler(TOOLBAR_SCHHDL_ID,TOOLBAR_Handler);
#endif
#ifdef PRIMER2
        if( CurrentScreenOrientation == V9 )
            {
            OrientationOffsetX[ CurrentScreenOrientation ] = +32;
            OrientationOffsetX[ CurrentScreenOrientation + 4 ] = +32;
            }
        if( CurrentScreenOrientation == V6 )
            {
            OrientationOffsetY[ CurrentScreenOrientation ] = +32;
            OrientationOffsetY[ CurrentScreenOrientation + 4 ] = +32;
            }
#endif
        LCD_SetRotateScreen(1);
        LCD_SetOffset(OFFSET_ON);
        LCD_DrawCharSetFilter( 0, APP_SCREEN_WIDTH, 0, APP_SCREEN_HEIGHT );
        POINTER_SetRectScreen();
        }

}

