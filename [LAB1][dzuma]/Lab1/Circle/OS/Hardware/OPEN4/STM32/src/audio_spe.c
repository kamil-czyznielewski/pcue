/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     audio_spe.c
* @brief    STw5094a audio codec init and management.
* @author   RemiC / YRT / FL
* @date     10/2008
*
* @date     09/2009
* @version  4.0 Platform = Open4 STM32/STw5094a
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define STw5094A_ADDR_WR   0xE2      /* addr for WRITE access*/
#define STw5094A_ADDR_RD   0xE3      /* addr for READ access*/

#define STw5094A_I2C_SPEED 100000

/* Maximum Timeout values for flags and events waiting loops. These timeouts are
   not based on accurate values, they just guarantee that the application will
   not remain stuck if the I2C communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */
#define STw5094A_I2C_FLAG_TIMEOUT         ((uint32_t)0x1000)
#define STw5094A_I2C_LONG_TIMEOUT         ((uint32_t)(10 * STw5094A_I2C_FLAG_TIMEOUT))

#define NB_REG 22
#define CONTROL_REGISTER_1 1
#define CONTROL_REGISTER_2 2
#define CONTROL_REGISTER_3 3
#define CONTROL_REGISTER_4 4
#define CONTROL_REGISTER_5 5
#define CONTROL_REGISTER_6 6
#define CONTROL_REGISTER_7 7
#define CONTROL_REGISTER_8 8
#define CONTROL_REGISTER_9 9
#define CONTROL_REGISTER_10 10
#define CONTROL_REGISTER_11 11
#define CONTROL_REGISTER_12 12
#define CONTROL_REGISTER_13 13
#define CONTROL_REGISTER_14 14
#define CONTROL_REGISTER_15 15
#define CONTROL_REGISTER_16 16
#define CONTROL_REGISTER_17 17
#define CONTROL_REGISTER_18 18
#define CONTROL_REGISTER_19 19
#define CONTROL_REGISTER_20 20
#define CONTROL_REGISTER_21 21

#define LOCAL_BUFFER_SIZE 1024          //size of the local buffer in 16 bits words

/* Private variables ---------------------------------------------------------*/
volatile u8 AUDIO_CODEC_CRs[22];
volatile s8   flagWrite_AUDIO_CODEC_CRs   = -1;
s8   AUDIO_Volume                         = 0;           // number of 2 dB steps for attenuation volume
bool AUDIO_SpeakerOn                      = 1;           // loudspeaker active or not
bool AUDIO_SpeakerOnOld                   = 0;           // flag for change detection
bool AUDIO_Mute                           = 0;           // sound (loudspeaker+headphones) active or not

const sound_type*  Audio_buffer;
vs32 Audio_buffer_index = 0;
volatile u16 Audio_buffer_size = 0;
volatile s16 Audio_buffer_local[LOCAL_BUFFER_SIZE];       // working buffer for MONO mode management
s32 Audio_buffer_local_size = LOCAL_BUFFER_SIZE;
AUDIO_PlaybackBuffer_Status bufferstatus_local;
volatile AUDIO_PlaybackBuffer_Status  audio_buffer_fill = LOW_EMPTY | HIGH_EMPTY ;

voice_type*  Voice_buffer;
volatile u16 Voice_buffer_index = 0;
volatile u16 Voice_buffer_size = 0;
volatile AUDIO_RecordBuffer_Status  voice_record_buffer_fill = EMPTY;

AUDIO_DeviceMode_enum AUDIO_DeviceMode = AUDIO_MODE;
AUDIO_Playback_status_enum AUDIO_Playback_status = NO_SOUND;
AUDIO_Recording_status_enum AUDIO_Recording_status = NO_RECORD;
AUDIO_Length_enum AUDIO_Length = LG_8_BITS;
AUDIO_Frequency_enum AUDIO_Frequency = FRQ_16KHZ;
AUDIO_Format_enum AUDIO_Format = MONO;

/* Setting of the prescaler depends on the PLL frequency..*/
#ifdef STM32F10X_CL
// Array for clocks configuration = 48 KHz, 44KHz, 22 KHz, 16 KHz, 8KHz
const u32 I2S_PreDiv2[5] = { RCC_PREDIV2_Div6, RCC_PREDIV2_Div11, RCC_PREDIV2_Div11,  RCC_PREDIV2_Div4, RCC_PREDIV2_Div12 };
const u32 I2S_Pll3Mul[5] = { RCC_PLL3Mul_20, RCC_PLL3Mul_20, RCC_PLL3Mul_10, RCC_PLL3Mul_20, RCC_PLL3Mul_20 };
const u16 I2S_PrescalerVal[5] = { 32,  18,  18, 144, 96 };
#else
// Array for I2S divider configuration = 18MHz, 24MHz, 36MHz, 48MHz, 72MHz
const u16 I2S_PrescalerVal[5] = { 0x111,  0x117, 0x123, 0x02F, 0x146 };
#endif

/* Private function prototypes -----------------------------------------------*/
void AUDIO_Init_I2C();
void AUDIO_Init_audio_mode( AUDIO_DeviceMode_enum mode, AUDIO_Length_enum length, AUDIO_Frequency_enum frequency, AUDIO_Format_enum format );
void AUDIO_Init_voice_mode( AUDIO_DeviceMode_enum mode );
s32  AUDIO_I2C_ReadMultByte( u8 ReadAddr, u8 NumByteToRead, u8* pBuffer );
void AUDIO_I2C_WriteMultByte( u8 WriteAddr, u8 NumByteToWrite, u8* pBuffer );


/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
*
*                                AUDIO_DeviceSoftwareReset
*
*******************************************************************************/
/**
*
*   Reset the audio codec.
*
**/
/******************************************************************************/
void AUDIO_DeviceSoftwareReset( void )
{
    u8 reset_value = 0x2;
    AUDIO_I2C_WriteMultByte( CONTROL_REGISTER_21, 1, &reset_value );
}


/*******************************************************************************
*
*                                AUDIO_UpdateRegisters
*
*******************************************************************************/
/**
*
*   Update one or all of codec registers.
*   flagWrite_AUDIO_CODEC_CRs contains number of the register to update.
*   If flag == 22, all registers are send to the codec.
*   Register values are copied from table AUDIO_CODEC_CRs.
*
**/
/******************************************************************************/
void AUDIO_UpdateRegisters( void )
{
    if ( ( flagWrite_AUDIO_CODEC_CRs > -1 ) && ( flagWrite_AUDIO_CODEC_CRs < 22 ) )
    {
        /* Write of the register passed in the flag parameter */
        AUDIO_I2C_WriteMultByte( flagWrite_AUDIO_CODEC_CRs, 1, ( u8* ) &AUDIO_CODEC_CRs[flagWrite_AUDIO_CODEC_CRs] );
    }
    else
    {
        if ( flagWrite_AUDIO_CODEC_CRs >= 22 )
        {
            /* Write all the 22 registers*/
            AUDIO_I2C_WriteMultByte( CONTROL_REGISTER_0, 22, ( u8* ) AUDIO_CODEC_CRs );
        }
    }
    flagWrite_AUDIO_CODEC_CRs = -1;
}

/*******************************************************************************
*
*                                AUDIO_SetClocks
*
*******************************************************************************/
/**
*
*   Set the clocks according to the sampling frequency required.
*
*   @param[in] frequency      :
*
**/
/******************************************************************************/
#ifdef STM32F10X_CL
void AUDIO_SetClocks( AUDIO_Frequency_enum frequency )
{
    // First stops the clock before configuration change

    /* Select internal clock as system clock source */
    RCC_SYSCLKConfig( RCC_SYSCLKSource_HSI );

    /* Disable PLL */
    RCC_PLLCmd( DISABLE );
    RCC_PLL3Cmd( DISABLE );
    RCC_PLL2Cmd( DISABLE );

    // Change the configuration accoding to IS2S spec (see Reference manual)
    RCC_PREDIV2Config( I2S_PreDiv2[frequency] );
    RCC_PLL3Config( I2S_Pll3Mul[frequency] );

    // Set the prescaler values for the I2S (audio codec) according to the PLLs
    *( u16* )SPI2_I2SPR = I2S_PrescalerVal[frequency];

    // Restart PLL2
    RCC_PLL2Cmd( ENABLE );
    /* Wait till PLL2 is ready */
    while ( ( RCC->CR & RCC_CR_PLL2RDY ) == 0 )
    {
    }

    /* Enable PLL */
    RCC_PLLCmd( ENABLE );
    /* Wait till PLL is ready */
    while ( ( RCC->CR & RCC_CR_PLLRDY ) == 0 )
    {
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig( RCC_SYSCLKSource_PLLCLK );
    /* Wait till PLL is used as system clock source */
    while ( ( RCC->CFGR & ( uint32_t )RCC_CFGR_SWS ) != ( uint32_t )0x08 )
    {
    }

    // Restart PLL3
    RCC_PLL3Cmd( ENABLE );
    /* Wait till PLL3 is ready */
    while ( ( RCC->CR & RCC_CR_PLL3RDY ) == 0 )
    {
    }

}
#endif

/* Public functions for CircleOS ---------------------------------------------*/

/*******************************************************************************
*
*                                AUDIO_Init
*
*******************************************************************************/
/**
*
*   General initialization of the STw5094a audio codec.
*   Only the I2C interface is activated.
*
*  @attention  This function must <b>NOT</b> be called by the user.
*
**/
/******************************************************************************/
void AUDIO_Init( void )
{

    /* Enable clocks */
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_I2C2, ENABLE );

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
                            RCC_APB2Periph_AFIO, ENABLE );

    /* Set the default values*/
    if ( fFirstStartup == FALSE )
    {
        AUDIO_Volume = ( ( UTIL_ReadBackupRegister( BKP_SYS2 ) ) & BKPMASK_S2_VOLUME ) >> 8 ;

        /* Restore the configuration    */
        AUDIO_SpeakerOn = ( ( UTIL_ReadBackupRegister( BKP_SYS2 ) ) & BKPMASK_S2_SPEAKER ) ? 1 : 0;
        AUDIO_SpeakerOnOld = AUDIO_SpeakerOn;
        AUDIO_SPEAKER_OnOff( AUDIO_SpeakerOn );
        AUDIO_Mute = ( ( UTIL_ReadBackupRegister( BKP_SYS2 ) ) & BKPMASK_S2_MUTE ) ? 1 : 0;
        AUDIO_BuzzerOn = ( ( UTIL_ReadBackupRegister( BKP_SYS2 ) ) & BKPMASK_S2_BUZZER ) ? 1 : 0;

    }
    else
    {
        AUDIO_SpeakerOn =  1;
        AUDIO_SpeakerOnOld = 1;
        AUDIO_Mute = FALSE;
        AUDIO_Volume = AUDIO_MIN_ATTENUATION;
        AUDIO_BuzzerOn = TRUE;
    }

    /* Init I2C */
    AUDIO_Init_I2C();

    /*--STEP 1 : device software reset*/
    AUDIO_DeviceSoftwareReset();

    /*--STEP 2 : read CR0 to CR21 to get default config*/
    AUDIO_I2C_ReadMultByte( CONTROL_REGISTER_0, 22, ( u8* ) AUDIO_CODEC_CRs );

    /* Init the default audio mode for playing 8 bits / 16 kHz audio sounds*/
//    AUDIO_Init_audio_mode(AUDIO_MODE, LG_8_BITS, FRQ_16KHZ, MONO);
    AUDIO_Init_voice_mode( VOICE_MODE );    // YRT20100324 : electrical pb on power on
    // if Loud Speaker driver ON

    // Write register now to power on the codec and his drivers
    AUDIO_UpdateRegisters();

    // Wait for power stabilization
    Delayms( 2000 );

    // Generate the 12 MHz clock for codec
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init( GPIOA, &GPIO_InitStructure );
    RCC_MCOConfig( RCC_MCO_HSE );
}

/*******************************************************************************
*
*                                AUDIO_Handler
*
*******************************************************************************/
/**
*
*  Called by the CircleOS scheduler to manage audio tasks.
*
*  @attention  This function must <b>NOT</b> be called by the user.
*
**/
/******************************************************************************/
void AUDIO_Handler( void )
{
    // Update codec registers, if requested
    AUDIO_UpdateRegisters();

    /* Apply configuration change, if necessary*/
    if ( AUDIO_SpeakerOn != AUDIO_SpeakerOnOld )
    {
        AUDIO_SPEAKER_OnOff( AUDIO_SpeakerOn );
        AUDIO_SpeakerOnOld = AUDIO_SpeakerOn;

        /* Update the default system toolbar*/
        TOOLBAR_SetDefaultToolbar();
    }

    return;
}


/*******************************************************************************
*
*                                AUDIO_CODEC_Init_I2C
*
********************************************************************************/
/**
*
*   Initialization of I2C communication
*   I2C is used to configure the STw5094a audio codec
*
**/
/********************************************************************************/
void AUDIO_Init_I2C( void )
{

    s32 i;
    GPIO_InitTypeDef  GPIO_InitStructure;
    I2C_InitTypeDef  I2C_InitStructure;

    /* GPIO configuration ---------- */
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE );

    /* Configure GPIO pins: SCL and SDA for I2C2 */
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init( GPIOB, &GPIO_InitStructure );

    /* I2C configuration ----------- */
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0xC0;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = STw5094A_I2C_SPEED;

    /* I2C Peripheral Enable */
    I2C_Cmd( I2C2, ENABLE );

    /* Apply I2C configuration after enabling it */
    I2C_Init( I2C2, &I2C_InitStructure );

}


/*******************************************************************************
*
*                             AUDIO_Init_audio_mode
*
*******************************************************************************/
/**
*
*   Initialization for Audio Mode use of the STw5094a codec:
*   - set STw5094a to Audio Mode via I2C
*   - enable STM32 I2S communication to send audio samples (SPI2/I2S2 port) in DMA mode
*
*   @param[in] mode      : AUDIO_MODE normal mode (one shot)
*                          AUDIO_CIRCULAR_MODE circular loop
*   @param[in] length    : 8 bits or 16 bits length sample
*   @param[in] frequency : 8 KHz, 16 KHz, 22 KHz, 44 KHz, 48 KHz sample
*   @param[in] format :    mono, stereo
*
*  Note:  values were determined according to the STw5094a datasheet
*
*  @attention  This function must <b>NOT</b> be called by the user.
*
**/
/******************************************************************************/
void AUDIO_Init_audio_mode( AUDIO_DeviceMode_enum mode, AUDIO_Length_enum length, AUDIO_Frequency_enum frequency, AUDIO_Format_enum format )
{
    s32 i = 0;
    DMA_InitTypeDef    DMA_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    I2S_InitTypeDef I2S_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;


    /* The MONO mode uses working buffer to dupplicate datas on the two channels*/
    /* and switch buffer half by half => uses DMA in circular mode*/
    /*KJ   if (format == MONO)*/
    /*KJ       mode = AUDIO_CIRCULAR_MODE;*/

    RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI2, ENABLE );
    I2S_Cmd( SPI2, DISABLE );

    /* Enable audio interface of STw5094a : set the proper config -------------*/

    /* MCLK or AUXCLK = 512 kHz; Voice Data Fs is 8 kHz ; Linear code ; B1 and B2 consecutive ; 8 bits time-slot*/
    AUDIO_CODEC_CRs[0]  = 0x00;

    /* Delayed data timing; CR2A connected to RX path ; TX path connected to DX; */
    /* PCM I/F enabled ; B1 channel selected; Normal operation*/
    AUDIO_CODEC_CRs[1]  = 0x14;

    /* Microphone inout "MIC1" Selected; MBIAS enable; Voice PreAmplifier gain = 22.5 dB*/
    AUDIO_CODEC_CRs[4]  = 0x6f;

    /* Voice Codec Receive High Pass filter enabled ; Sidetone gain = -19.5 dB (voice -> HP)*/
    AUDIO_CODEC_CRs[5]  = 0x17;

    /* !MUT + PLS (LoudSpeaker) + PHL/PHR (Headphone)+ SE + !RTE (tone)*/
    AUDIO_CODEC_CRs[6]  = ( AUDIO_SpeakerOn ? 0x10 : 0 ) + 0x0c + 0x02 ;

    /* MUT*/
    if ( AUDIO_Mute == TRUE )
    {
        AUDIO_CODEC_CRs[6] |= 0x20;
    }
    else
    {
        AUDIO_CODEC_CRs[6] &= 0xDF;
    }

    s32 hp_setting = ( AUDIO_Volume + 2 ) * 2;
    if ( hp_setting > 0x14 )
        hp_setting = 0x14;

    /* Apply new volume to codec */
    AUDIO_CODEC_CRs[7] = AUDIO_Volume;      /* loudspeaker gain ( +6 / -24 dB)*/
    AUDIO_CODEC_CRs[8] = hp_setting ;       /* left HeadPhones gain  ( +0 / -40 dB)*/
    AUDIO_CODEC_CRs[9] = hp_setting ;       /* right HeadPhones gain ( +0 / -40 dB)*/

    /* Tone generator :  F1 selected, tone gain = -24db, square*/
    AUDIO_CODEC_CRs[12] = 0x84;

    /* Tone generator : F1/F2 frequency = 445 Hz*/
    AUDIO_CODEC_CRs[13] = 89;
    AUDIO_CODEC_CRs[14] = 89;

    AUDIO_CODEC_CRs[16] = 0x08; /*KJ20090320 - Changed from 0x00 to 0x08 for Non-Delayed data.*/

    /* Headphone common driver = 1.35V + enable */
#ifdef STM32F10X_CL
    AUDIO_CODEC_CRs[18] = 0x61;     /* AMCK between 14 and 19 MHz*/
#else
    AUDIO_CODEC_CRs[18] = 0x60;     /* AMCK between 9 and et 14 MHz*/
#endif

    /* Audio mode + power ON + AMCK clock for tone only*/
    AUDIO_CODEC_CRs[21] = 0x61;

    flagWrite_AUDIO_CODEC_CRs = 22;


    /* Init I2S communication to send data on Audio interface -----------------*/

    /* Enable the DMA1 Channel 5 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init( &NVIC_InitStructure );

    /* DMA1 Channel4 configuration ----------------------------------------------*/
    RCC_AHBPeriphClockCmd( RCC_AHBPeriph_DMA1, ENABLE );
    DMA_DeInit( DMA1_Channel5 );
    DMA_InitStructure.DMA_PeripheralBaseAddr = ( u32 )&SPI2->DR;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    if ( ( mode == AUDIO_CIRCULAR_MODE ) || format == MONO ) /*/ Changed KJ 27May-09*/
        DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    else
        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init( DMA1_Channel5, &DMA_InitStructure );

    DMA_ITConfig( DMA1_Channel5, DMA_IT_TC | DMA_IT_HT, ENABLE );

    /* GPIO configuration for I2S2 port*/

    /* Configure SPI2 pins: SD*/
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init( GPIOB, &GPIO_InitStructure );

    /* Configure SPI2 pins: SCK and WS*/
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12 | GPIO_Pin_13 ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init( GPIOB, &GPIO_InitStructure );

    /*-- Configure I2S --*/
    /* I2S peripheral configuration*/
    I2S_InitStructure.I2S_Standard   = I2S_Standard_MSB;

    /* I2S audio samples length - Only support 8 and 16 bit - sound quality isn't anyway better out...*/
    I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
    I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;

    /* IS2S audio samples frequency*/
    switch ( frequency ) /*/ KJ20090320 - Changed Selection of FRQ*/
    {
    case FRQ_8KHZ : I2S_InitStructure.I2S_AudioFreq  = I2S_AudioFreq_8k;
        break;
    case FRQ_16KHZ : I2S_InitStructure.I2S_AudioFreq  = I2S_AudioFreq_16k;
        break;
    case FRQ_22KHZ : I2S_InitStructure.I2S_AudioFreq  = I2S_AudioFreq_22k;
        break;
    case FRQ_44KHZ : I2S_InitStructure.I2S_AudioFreq  = I2S_AudioFreq_44k;
        break;
    case FRQ_48KHZ : I2S_InitStructure.I2S_AudioFreq  = I2S_AudioFreq_48k;
        break;
    default:         I2S_InitStructure.I2S_AudioFreq  = I2S_AudioFreq_8k;
    }

    I2S_InitStructure.I2S_CPOL       = I2S_CPOL_Low;
    I2S_InitStructure.I2S_Mode       = I2S_Mode_MasterTx;

    /* Set the I2S prescaler value according to the PLL value*/
//    UTIL_SetPll( UTIL_GetPll() );  YRT091208 The prescaler is already calculated in the I2S_Init function

    I2S_Init( SPI2, &I2S_InitStructure );

#ifdef STM32F10X_CL
    /* Set the I2S prescaler value according to the PLL value*/
    AUDIO_SetClocks( frequency );
#endif

    Audio_buffer_index = 0;
    AUDIO_Playback_status = NO_SOUND;

    /* Memorize new audio format values*/
    AUDIO_Length = length;
    AUDIO_Frequency = frequency;
    AUDIO_Format = format;

    /*Buffers are supposed to be empty here*/
    audio_buffer_fill = LOW_EMPTY | HIGH_EMPTY ;

    /* Enable the I2S2*/
    I2S_Cmd( SPI2, ENABLE );

    /* Audio mode ready*/
}

/*******************************************************************************
*
*                             AUDIO_Init_voice_mode
*
*******************************************************************************/
/**
*
*   Initialization for Voice Mode use of the STw5094a codec:
*   - set STw5094a to Voice Mode via I2C
*
*   @param[in] mode      : VOICE_MODE normal mode (one shot)
*                          VOICE_CIRCULAR_MODE circular loop
*
*   Note :    Mode supported : 16 BITS / 16 KHz / MONO
*
*  @attention  This function must <b>NOT</b> be called by the user.
*
**/
/******************************************************************************/
void AUDIO_Init_voice_mode( AUDIO_DeviceMode_enum mode )
{
    s32 i = 0;
    GPIO_InitTypeDef GPIO_InitStructure;
    I2S_InitTypeDef I2S_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    DMA_InitTypeDef    DMA_InitStructure;

    RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI2 , ENABLE );
    I2S_Cmd( SPI2, DISABLE );

    /* Enable audio interface of STw5094a : set the proper config -------------*/

    /* MCLK or AUXCLK = 512 kHz; Voice Data Fs is 16 kHz ; Linear code ; B1 and B2 consecutive ; 8 bits time-slot*/
    AUDIO_CODEC_CRs[0]  = 0x20; /*/0x20 16khz / 0x00 8Khz*/

    /* Delayed data timing; CR2A connected to RX path ; TX path connected to DX; */
    /* PCM I/F enabled ; B1 channel selected; Normal operation*/
    AUDIO_CODEC_CRs[1]  = 0x14;

    /* Microphone inout "MIC1" Selected; MBIAS enable; Voice PreAmplifier gain = 22.5 dB*/
    AUDIO_CODEC_CRs[4]  = 0x6f;

    /* Voice Codec Receive High Pass filter enabled (HPB); Codec internal sidetone enabled (SI)*/
    /* Sidetone gain = -15.5 dB (voice -> HP)*/
    AUDIO_CODEC_CRs[5]  = 0x13;

    /* !PLS (LoudSpeaker) + !PHL/PHR (Headphone)+ !SE + !RTE (tone)*/
    AUDIO_CODEC_CRs[6]  = 0; /*record, no sound  (AUDIO_SpeakerOn?0x10:0) + 0x0c; */

    /* MUT*/
    if ( AUDIO_Mute == TRUE )
    {
        AUDIO_CODEC_CRs[6] |= 0x20;
    }
    else
    {
        AUDIO_CODEC_CRs[6] &= 0xDF;
    }

    /* Apply volume to codec */
    s32 hp_setting = ( AUDIO_Volume + 2 ) * 2;
    if ( hp_setting > 0x14 )
        hp_setting = 0x14;

    /* Apply new volume to codec */
    AUDIO_CODEC_CRs[7] = AUDIO_Volume;      /* loudspeaker gain ( +6 / -24 dB)*/
    AUDIO_CODEC_CRs[8] = hp_setting ;       /* left HeadPhones gain  ( +0 / -40 dB)*/
    AUDIO_CODEC_CRs[9] = hp_setting ;       /* right HeadPhones gain ( +0 / -40 dB)*/

    /* Tone generator :  F1 and F2 muted, tone gain = 0 db, square, no output*/
    AUDIO_CODEC_CRs[12] = 0x00;

    /* Tone generator : F1/F2 frequency = 445 Hz*/
    AUDIO_CODEC_CRs[13]  = AUDIO_CODEC_CRs[14]  = 0x0;

    /* Headphone common driver = 1.35V + enable / AMCK between 9 et 14 MHz*/
    AUDIO_CODEC_CRs[18] = 0x60;

    /* Voice mode + power ON + AUXCLK clock for tone only*/
    AUDIO_CODEC_CRs[21] = 0x01;

    flagWrite_AUDIO_CODEC_CRs = 22;

    /* Init I2S communication to send data on Audio interface -----------------*/

    /* Enable the DMA1 Channel 4 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init( &NVIC_InitStructure );

    /* DMA1 Channel4 configuration ----------------------------------------------*/
    RCC_AHBPeriphClockCmd( RCC_AHBPeriph_DMA1, ENABLE );
    DMA_DeInit( DMA1_Channel4 );
    DMA_InitStructure.DMA_PeripheralBaseAddr = ( u32 )&SPI2->DR;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    if ( mode == VOICE_CIRCULAR_MODE )
        DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    else
        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init( DMA1_Channel4, &DMA_InitStructure );

    DMA_ITConfig( DMA1_Channel4, DMA_IT_TC | DMA_IT_HT, ENABLE );

    /* GPIO configuration for SPI2=I2S2=PCM port*/

    /* Configure SPI2 pins: SD*/
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_OD;
    GPIO_Init( GPIOB, &GPIO_InitStructure );

    /* Configure SPI2 pins: SCK and WS*/
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12 | GPIO_Pin_13 ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init( GPIOB, &GPIO_InitStructure );

    /*-- Configure I2S --*/
    /* I2S peripheral configuration, set PCM standard*/
    I2S_InitStructure.I2S_Standard = I2S_Standard_PCMShort;

    /* I2S audio samples length*/
    I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16bextended; /* We only support this mode..*/

    I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;

    /* IS2S audio samples frequency*/
    I2S_InitStructure.I2S_AudioFreq  = I2S_AudioFreq_16k; /* We only support 16bit due to MCLK 1CH x 32Bit x 16Khz = 512KhzMCLK*/
    I2S_InitStructure.I2S_CPOL       = I2S_CPOL_Low;
    I2S_InitStructure.I2S_Mode       = I2S_Mode_MasterRx;
    I2S_Init( SPI2, &I2S_InitStructure );

    /* Set the I2S prescaler value according to the PLL value*/
#ifdef STM32F10X_CL
    /* Set the clocks according to the sample frequency required */
    UTIL_SetPll( SPEED_VERY_HIGH );
    AUDIO_SetClocks( FRQ_16KHZ );
#else
    UTIL_SetPll( UTIL_GetPll() );
#endif

    Voice_buffer_index = 0;
    AUDIO_Recording_status = NO_RECORD;

    /* Memorize new audio format values*/
    AUDIO_Length = LG_16_BITS;
    AUDIO_Frequency = FRQ_16KHZ;
    AUDIO_Format = MONO;

    /* Enable the I2S2*/
    I2S_Cmd( SPI2, ENABLE );

    /* Voice mode ready*/
}


/*******************************************************************************
*
*                                AUDIO_Shutdown
*
*******************************************************************************/
/**
*
*  Called by the CircleOS scheduler to shutdown the audio codec.
*
*  @attention  This function must <b>NOT</b> be called by the user.
*
**/
/******************************************************************************/
void AUDIO_Shutdown( void )
{
    /* Stop IT SPI*/
    I2S_Cmd( SPI2, DISABLE );

    /* Reset and power down the codec*/
    AUDIO_DeviceSoftwareReset();

    /* Stop I2C*/
    I2C_Cmd( I2C2, DISABLE );
}

/*******************************************************************************
*
*                                AUDIO_Welcome_Msg
*
********************************************************************************/
/**
*
*   Play the Welcome message to Open4 Primer.
*
**/
/********************************************************************************/
const u8 welcome_16K_8 [] =
{
#include "welcome_stm32_16k_8b.h"
};

void AUDIO_Welcome_Msg( void )
{
    AUDIO_SetMode( AUDIO_MODE, LG_8_BITS, FRQ_16KHZ, MONO );
    AUDIO_Play( ( sound_type* )welcome_16K_8, sizeof welcome_16K_8 );
}

/*******************************************************************************
*
*                            AUDIO_I2C_Read_Register
*
*******************************************************************************/
/**
*
*   Reads a data byte from one of STw5094A configuration registers.
*
*   @param[in] register_to_read : Specify the location where to read (the register number)
*   @return    unsigned byte    : The data read
*
*  @attention  This function must <b>NOT</b> be called by the user.
*
**/
/******************************************************************************/
u8 AUDIO_I2C_Read_Register( u8 register_to_read )
{
    u8 val = 0;
    AUDIO_I2C_ReadMultByte( register_to_read, 1, &val );
    return val;
}

/*******************************************************************************
*
*                           AUDIO_I2C_WriteRegister
*
*******************************************************************************/
/**
*
*   Send a data byte to one of STw5094A configuration registers.
*
*   @param[in] register_to_write : specifies the location where to write (the register number)
*   @param[in] data_to_write     : the data that you want to write
*   @return   : None
*
*  @attention  This function must <b>NOT</b> be called by the user.
*
**/
/******************************************************************************/
void AUDIO_I2C_WriteRegister( u8 register_to_write, u8 data_to_write )
{
    AUDIO_I2C_WriteMultByte( register_to_write, 1, &data_to_write );
}

/*******************************************************************************
*
*                            AUDIO_I2C_WriteMultByte
*
*******************************************************************************/
/**
*
*   Sends a data buffer to STw5094A configuration registers.
*
*   See STw5094A datasheet for a description of STw5094A multi-byte mode.
*
*   @param[in] WriteAddr      : first register address
*   @param[in] pBuffer        : address of the buffer to send
*   @param[in] NumByteToWrite : buffer size (max = 22)
*   @return   : None
*
*  @attention  This function must <b>NOT</b> be called by the user.
*
**/
/******************************************************************************/
void AUDIO_I2C_WriteMultByte( u8 WriteAddr, u8 NumByteToWrite, u8* pBuffer )
{
    s32 i;
    vu32 dr_temp ;
    u32 TimeOut;

    /* Send START condition */
    I2C_GenerateSTART( I2C2, ENABLE );

    /* Test on EV5 and clear it */
    TimeOut = STw5094A_I2C_LONG_TIMEOUT;
    while ( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_MODE_SELECT ) )
    {
        if ( I2C2->SR1 & I2C_FLAG_RXNE )
            dr_temp = I2C2->DR;

        if ( ( TimeOut-- ) == 0 )
            return;
    }

    /* Send STw5094A address for write */
    I2C_Send7bitAddress( I2C2, STw5094A_ADDR_WR, I2C_Direction_Transmitter );

    /* Test on EV6 and clear it */
    TimeOut = STw5094A_I2C_LONG_TIMEOUT;
    while ( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) )
    {
        if ( ( TimeOut-- ) == 0 )
            return;
    }

    /* Send the STw5094A's internal address to write to */
    I2C_SendData( I2C2, WriteAddr );


    /* Test on EV8 and clear it */
    TimeOut = STw5094A_I2C_LONG_TIMEOUT;
    while ( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED ) )
    {
        if ( ( TimeOut-- ) == 0 )
            return;
    }

    while ( NumByteToWrite ) /* uses the STw5094A's addr. register auto-increment feature*/
    {
        /* Send the byte to be written */
        I2C_SendData( I2C2, * pBuffer );

        /* Test on EV8 and clear it */
        TimeOut = STw5094A_I2C_LONG_TIMEOUT;
        while ( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED ) )
        {
            if ( ( TimeOut-- ) == 0 )
                return;
        }

        if ( NumByteToWrite == 1 )
            { I2C_GenerateSTOP( I2C2, ENABLE ); } /* STOP */

        pBuffer++;
        NumByteToWrite--;
    }
    for ( i = 0 ; i < 500 ; i++ )    { ; }
}

/*******************************************************************************
*
*                            AUDIO_I2C_ReadMultByte
*
*******************************************************************************/
/**
*
*   Reads "NumByteToRead" bytes from STw5094A configuration registers and
*   stores them in "pBuffer".
*
*   See STw5094A datasheet for a description of STw5094A multi-byte mode.
*
*   @param[in] ReadAddr      : first register address
*   @param[in] pBuffer       : address of the buffer to store data
*   @param[in] NumByteToRead : buffer size (max = 22)
*   @return   : none
*
*  @attention  This function must <b>NOT</b> be called by the user.
*
**/
/******************************************************************************/
s32 AUDIO_I2C_ReadMultByte( u8 ReadAddr, u8 NumByteToRead, u8* pBuffer )
{
    s32 i;
    vu32 dr_temp ;
    u32 TimeOut;

    /* Send START condition */
    I2C_GenerateSTART( I2C2, ENABLE );

    /* Test on EV5 and clear it */
    TimeOut = STw5094A_I2C_LONG_TIMEOUT;
    while ( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_MODE_SELECT ) )
    {
        if ( I2C2->SR1 & I2C_FLAG_RXNE )
            dr_temp = I2C2->DR;
    }

    /* Send STw5094A address for write */
    I2C_Send7bitAddress( I2C2, STw5094A_ADDR_WR, I2C_Direction_Transmitter );


    /* Test on EV6 and clear it */
    TimeOut = STw5094A_I2C_LONG_TIMEOUT;
    while ( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) );

    /* Clear EV6 by setting again the PE bit */
    I2C_Cmd( I2C2, ENABLE );

    /* Send the STw5094A's internal address to write to */
    I2C_SendData( I2C2, ReadAddr );

    /* Test on EV8 and clear it */
    TimeOut = STw5094A_I2C_LONG_TIMEOUT;
    while ( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED ) );

    /* Send START condition a second time */
    I2C_GenerateSTART( I2C2, ENABLE );

    /* Test on EV5 and clear it */
    TimeOut = STw5094A_I2C_LONG_TIMEOUT;
    while ( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_MODE_SELECT ) );

    /* Send STw5094A address for read */
    I2C_Send7bitAddress( I2C2, STw5094A_ADDR_RD, I2C_Direction_Receiver );

    /* Test on EV6 and clear it */
    TimeOut = STw5094A_I2C_LONG_TIMEOUT;
    while ( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED ) );

    /* While there is data to be read */
    while ( NumByteToRead )
    {
        if ( NumByteToRead == 1 )
        {
            /* Disable Acknowledgement */
            I2C_AcknowledgeConfig( I2C2, DISABLE );

            /* Send STOP Condition */
            I2C_GenerateSTOP( I2C2, ENABLE );
        }

        /* Test on EV7 and clear it */
        TimeOut = STw5094A_I2C_LONG_TIMEOUT;
        if ( I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED ) )
        {
            /* Read a byte from the EEPROM */
            *pBuffer = I2C_ReceiveData( I2C2 );

            /* Point to the next location where the byte read will be saved */
            pBuffer++;

            /* Decrement the read bytes counter */
            NumByteToRead--;
        }
        else
        {
            if ( ( TimeOut-- ) == 0 )
                return;
        }

    }

    /* Enable Acknowledgement to be ready for another reception */
    I2C_AcknowledgeConfig( I2C2, ENABLE );

    for ( i = 0 ; i < 500 ; i++ )    { ; }
}


/*******************************************************************************
*
*                            AUDIO_BUZZER_SetToneFrequency
*
*******************************************************************************/
/**
*
*   Set the frequency of the audio tone generator for the audio buzzer.
*
*   @param[in] f  : frequency value 0 to 3800 Hz
*                   => conversion to register value value O to 255
*                      according to the table provided by the STw5094A datasheet
*
**/
/******************************************************************************/
void AUDIO_BUZZER_SetToneFrequency( u16 freq )
{
    u16 valreg;

    if ( freq < 250 )
        valreg = ( 256 * freq ) / 1000;
    else
    {
        if ( freq < 750 )
            valreg = ( ( 128 * freq ) / 1000 ) + 32;
        else
        {
            if ( freq < 1750 )
                valreg = ( ( 64 * freq ) / 1000 ) + 80;
            else
            {
                if ( freq < 3720 )
                    valreg = ( ( 32 * freq ) / 1000 ) + 136;
                else
                    valreg = 255;
            }
        }
    }

    /* Set frequency to */
    AUDIO_CODEC_CRs[13] = valreg;
    AUDIO_CODEC_CRs[14] = valreg;

    /* Update register (to be done by the tim2 irq)*/
    SET_FLAG_WRITE_CODEC_CRS( 13 );
}

/*******************************************************************************
*
*                            AUDIO_BUZZER_OnOff
*
*******************************************************************************/
/**
*
*   Set the RTE switch of the audio codec ON or OFF, in order to mute
*   or not the audio buzzer.
*   If RTE = ON, the buzzer is active through Loudspeaker and Headphones,
*   depending of the MUT switch position
*
*   @param[in] mode : ON or OFF
*
**/
/******************************************************************************/
void AUDIO_BUZZER_OnOff( ON_OFF_enum mode )
{
    if ( mode != OFF )
        AUDIO_CODEC_CRs[6] |= 0x01;
    else
        AUDIO_CODEC_CRs[6] &= 0xFE;

    /* Update register (to be done by the tim2 irq)*/
    SET_FLAG_WRITE_CODEC_CRS( 6 );
}

/*******************************************************************************
*
*                            AUDIO_Set_Volume
*
*******************************************************************************/
/**
*
*
**/
/******************************************************************************/
void AUDIO_Set_Volume( void )
{
    s32 hp_setting = ( AUDIO_Volume + 2 ) * 2;
    if ( hp_setting > 0x14 )
        hp_setting = 0x14;

    /* Apply new volume to codec */
    AUDIO_CODEC_CRs[7] = AUDIO_Volume;      /* loudspeaker gain ( +6 / -24 dB)*/
    AUDIO_CODEC_CRs[8] = hp_setting ;       /* left HeadPhones gain  ( +0 / -40 dB)*/
    AUDIO_CODEC_CRs[9] = hp_setting ;       /* right HeadPhones gain ( +0 / -40 dB)*/

    /* Update all registers (to be done by the tim2 irq)*/
    SET_FLAG_WRITE_CODEC_CRS( 22 );
}

/*******************************************************************************
*
*                                AUDIO_Cpy_Mono
*
********************************************************************************/
/**
*
*   Copy Mono data to small buffer, set both Left+Right Channel to samme value
*
**/
/********************************************************************************/
void AUDIO_Cpy_Mono( void )
{
    volatile u16 i;
    s16  temp;

    /* Stop if end of original buffer */
    if ( Audio_buffer_index >= Audio_buffer_size )
    {
        if ( AUDIO_DeviceMode == AUDIO_MODE ) /* KJ 27may-2009*/
            AUDIO_Playback_Stop();
    }

    /* End of the first half working buffer*/
    if ( bufferstatus_local & LOW_EMPTY )
    {
        if ( AUDIO_Length == LG_16_BITS )
        {
            for ( i = 0 ; i < ( ( Audio_buffer_local_size / 4 ) - 1 ) ; i = i + 2 )
            {
                temp = *( s16* )( Audio_buffer + Audio_buffer_index );
                Audio_buffer_local[i + 1] = Audio_buffer_local[i] = temp;
                Audio_buffer_index += 2; /* 16 bit*/
            }
        }
        else
        {
            for ( i = 0; i < ( ( Audio_buffer_local_size / 4 ) - 1 ); i = i + 2 )
            {
                temp = ( ( s16 )( ( *( Audio_buffer + Audio_buffer_index ) ^ 0x80 ) ) ) << 8;
                Audio_buffer_local[i + 1] = Audio_buffer_local[i] = temp;
                Audio_buffer_index++; /* 8 bit      */
            }
        }
        bufferstatus_local &= ~LOW_EMPTY;
    }

    /* End of the second half working buffer*/
    if ( bufferstatus_local & HIGH_EMPTY )
    {
        if ( AUDIO_Length == LG_16_BITS )
        {
            for ( i = ( Audio_buffer_local_size / 4 ) ; i < ( Audio_buffer_local_size / 2 ); i = i + 2 )
            {
                temp = *( s16* )( Audio_buffer + Audio_buffer_index );
                Audio_buffer_local[i + 1] = Audio_buffer_local[i] = temp;
                Audio_buffer_index += 2; /* 16 bit*/
            }
        }
        else
        {
            for ( i = ( Audio_buffer_local_size / 4 ); i < ( ( Audio_buffer_local_size ) / 2 ) - 1; i = i + 2 )
            {
                temp = ( ( s16 )( ( *( Audio_buffer + Audio_buffer_index ) ^ 0x80 ) ) ) << 8;
                Audio_buffer_local[i + 1] = Audio_buffer_local[i] = temp;
                Audio_buffer_index++; /* 8 bit      */
            }
        }
        bufferstatus_local &= ~HIGH_EMPTY;
    }

    if ( Audio_buffer_index == ( Audio_buffer_size / 2 ) )
    {
        audio_buffer_fill |= LOW_EMPTY;
        if ( audio_buffer_fill & HIGH_EMPTY )
        {
            if ( AUDIO_DeviceMode == AUDIO_MODE ) /* KJ 27may-2009*/
                AUDIO_Playback_Stop();
        }
    }
    if ( Audio_buffer_index >= Audio_buffer_size )
    {
        audio_buffer_fill |= HIGH_EMPTY;
        Audio_buffer_index = 0;
        if ( audio_buffer_fill & LOW_EMPTY )
        {
            if ( AUDIO_DeviceMode == AUDIO_MODE ) /* KJ 27may-2009*/
                AUDIO_Playback_Stop();
        }
    }
}

/// @endcond

/* Public functions ----------------------------------------------------------*/


/*******************************************************************************
*
*                                AUDIO_SetMode
*
********************************************************************************/
/**
*
*   Set new codec mode. Mode can be :
*   AUDIO_MODE
*   VOICE_MODE
*
*   @param[in]  mode      : the new codec mode.
*   @param[in]  length    : the length of the samples (8, 16 bits...).
*   @param[in]  frequency : frequency of the samples (8, 16 kHz...).
*   @param[in]  format    : format of the samples (Mono, Stereo).
*
**/
/********************************************************************************/
void AUDIO_SetMode( AUDIO_DeviceMode_enum mode,
                    AUDIO_Length_enum length,
                    AUDIO_Frequency_enum frequency,
                    AUDIO_Format_enum format )
{
    if ( ( mode == VOICE_MODE ) || ( mode == VOICE_CIRCULAR_MODE ) )
        AUDIO_Init_voice_mode( mode );
    else
        AUDIO_Init_audio_mode( mode, length, frequency, format );

    AUDIO_DeviceMode = mode;
}


/*******************************************************************************
*
*                                AUDIO_GetMode
*
********************************************************************************/
/**
*
*   Get the current codec mode. Mode can be :
*   AUDIO_MODE
*   VOICE_MODE
*
*   @return  The current codec mode.
*
**/
/********************************************************************************/
AUDIO_DeviceMode_enum AUDIO_GetMode( void )
{
    return AUDIO_DeviceMode;
}

/*******************************************************************************
*
*                                AUDIO_Play
*
********************************************************************************/
/**
*
*   Issues audio samples (stored in buffer) to the audio codec via I2S.
*
*   @param[in]  buffer      : address of the buffer to play.
*   @param[in]  size        : size of the buffer.
*
*   Note :   In mono mode, stop is managed by AUDIO_Cpy_Mono().
*            In stereo mode, it must be managed by the application.
*
**/
/********************************************************************************/
void AUDIO_Play( const sound_type* buffer, s32 size )
{
    audio_buffer_fill =  LOW_EMPTY | HIGH_EMPTY;
    bufferstatus_local =  LOW_EMPTY | HIGH_EMPTY;

    if ( AUDIO_Format == STEREO )
    {
        /* Uses the original buffer*/
        DMA1_Channel5->CMAR = ( u32 )buffer;         // Set the buffer
        DMA1_Channel5->CNDTR = size;                 /* Set the size */
    }
    else
    {
        /* Uses the local buffer*/
        if ( AUDIO_Length == LG_16_BITS )
            size *= 2;
        Audio_buffer = ( sound_type* ) buffer;       /* Copy the pointer to real databuffer/source*/
        Audio_buffer_index = 0;                      /* Copy the pointer to real databuffer/source*/
        Audio_buffer_size = size;                    /* Size to the real databuffer/source*/
        AUDIO_Cpy_Mono();
        DMA1_Channel5->CMAR = ( u32 )Audio_buffer_local;        /* Set the buffer*/
        DMA1_Channel5->CNDTR = ( ( Audio_buffer_local_size ) / 2 ); /* Set the size*/
    }

    DMA_Cmd( DMA1_Channel5, ENABLE );                 /* Enable DMA Channel for Audio*/
    SPI_I2S_DMACmd( SPI2, SPI_I2S_DMAReq_Tx, ENABLE ); /* Enable I2S DMA REQ.*/
    AUDIO_Playback_status = IS_PLAYING;
}

/*******************************************************************************
*
*                                AUDIO_Playback_Stop
*
********************************************************************************/
/**
*
*   Stop the playback by stopping the DMA transfer.
*
**/
/********************************************************************************/
void AUDIO_Playback_Stop( void )
{
    vs32 i;

    DMA_Cmd( DMA1_Channel5, DISABLE );                  /* Disable DMA Channel for Audio*/
    SPI_I2S_DMACmd( SPI2, SPI_I2S_DMAReq_Tx, DISABLE ); /* Disable I2S DMA REQ.*/

    AUDIO_Playback_status = NO_SOUND;

    /* Shutdwon codec in order to avoid non expected voices*/
    AUDIO_WriteRegister( 21, 0x60 );
    for ( i = 0; i < 10000 ; i++ ) {;}

    /* Wake up codec*/
    AUDIO_WriteRegister( 21, 0x61 );

}


/*******************************************************************************
*
*                                AUDIO_Record
*
********************************************************************************/
/**
*
*   Stores audio samples to the buffer from the audio codec via I2S.
*
*   @param[in]  buffer      : address of the buffer to record into.
*   @param[in]  size        : size of the buffer.
**/
/********************************************************************************/
void AUDIO_Record( sound_type* buffer, s32 size )
{
    /* Initialize the recording buffer*/
    Voice_buffer = ( voice_type* ) buffer;
    Voice_buffer_size = size;
    Voice_buffer_index = 0;
    voice_record_buffer_fill = EMPTY;

    /* Inform that actual data tranfer is processed in the interrupt handler*/
    AUDIO_Recording_status = IS_RECORDING;
    DMA1_Channel4->CMAR = ( u32 )buffer;                 /* Set the buffer*/
    DMA1_Channel4->CNDTR = size;                         /* Set the size*/
    DMA_Cmd( DMA1_Channel4, ENABLE );                    /* Enable DMA Channel for Audio*/
    SPI_I2S_DMACmd( SPI2, SPI_I2S_DMAReq_Rx, ENABLE );   /* Enable I2S DMA REQ.*/
}

/*******************************************************************************
*
*                                AUDIO_Record_Stop
*
********************************************************************************/
/**
*
*   Stop the record by stopping the DMA transfer.
*
**/
/********************************************************************************/
void AUDIO_Record_Stop( void )
{
    DMA_Cmd( DMA1_Channel4, DISABLE );                  /* Disable DMA Channel for Audio */
    SPI_I2S_DMACmd( SPI2, SPI_I2S_DMAReq_Rx, DISABLE ); /* Disable I2S DMA REQ.*/
    AUDIO_Recording_status = NO_RECORD;
}

/*******************************************************************************
*
*                            AUDIO_Playback_GetStatus
*
*******************************************************************************/
/**
*
*   Gets the status of playback mode.
*
*   @return status : NO_SOUND, IS_PLAYING
*
**/
/******************************************************************************/
AUDIO_Playback_status_enum AUDIO_Playback_GetStatus( void )
{
    return AUDIO_Playback_status;
}

/*******************************************************************************
*
*                            AUDIO_PlaybackBuffer_GetStatus
*
*******************************************************************************/
/**
*
*   Gets the status of Playback buffer.
*
*   @param[in]  value : 0 = get the current status, else reset the flag
*                        passed in parameter .
*
*   @return status : FULL=0, LOW_EMPTY=1, HIGH_EMPTY=2
*
**/
/******************************************************************************/
AUDIO_PlaybackBuffer_Status AUDIO_PlaybackBuffer_GetStatus( AUDIO_PlaybackBuffer_Status value )
{
    if ( value )
        audio_buffer_fill &= ~value;
    return audio_buffer_fill;
}

/*******************************************************************************
*
*                            AUDIO_Record_Buffer_GetStatus
*
*******************************************************************************/
/**
*
*   Gets the status of Record buffer.
*
*   @param[in]  value : 0 = get the current status, else reset the flag
*                        passed in parameter.
*
*   @return status : EMPTY=0, LOW_FULL=1, HIGH_FULL=2
*
**/
/******************************************************************************/
AUDIO_RecordBuffer_Status AUDIO_RecordBuffer_GetStatus( AUDIO_RecordBuffer_Status value )
{
    if ( value )
        voice_record_buffer_fill &= ~value;
    return voice_record_buffer_fill;
}

/*******************************************************************************
*
*                            AUDIO_Recording_GetStatus
*
*******************************************************************************/
/**
*
*   Gets the status of recording mode.
*
*   @return status  : NO_RECORD, IS_RECORDING
*
**/
/******************************************************************************/
AUDIO_Recording_status_enum AUDIO_Recording_GetStatus( void )
{
    return AUDIO_Recording_status;
}

/*******************************************************************************
*
*                            AUDIO_SPEAKER_OnOff
*
*******************************************************************************/
/**
*
*   Set the PLS switch of the audio codec ON or OFF, in order to mute
*   or not the loudspeaker.
*   If PLS = ON, the Loudspeaker is active for audio and buzzer.
*
*   @param[in] mode : ON or OFF
*
**/
/******************************************************************************/
void AUDIO_SPEAKER_OnOff( ON_OFF_enum mode )
{
    if ( mode != OFF )
    {
        AUDIO_CODEC_CRs[6] |= 0x10;
        AUDIO_SpeakerOn = 1;
    }
    else
    {
        AUDIO_CODEC_CRs[6] &= 0xEF;
        AUDIO_SpeakerOn = 0;
    }

    /* Update register */
    SET_FLAG_WRITE_CODEC_CRS( 6 );
}

/*******************************************************************************
*
*                            AUDIO_MUTE_OnOff
*
*******************************************************************************/
/**
*
*   Set the MUT switch of the audio codec ON or OFF.
*   If MUT = ON, both buzzer, Loudspeaker and Headphones are cut off.
*
*   @param[in] mode : ON or OFF
*
**/
/******************************************************************************/
void AUDIO_MUTE_OnOff( ON_OFF_enum mode )
{
    if ( mode != OFF )
    {
        AUDIO_CODEC_CRs[6] |= 0x20;
        AUDIO_Mute = TRUE;
    }
    else
    {
        AUDIO_CODEC_CRs[6] &= 0xDF;
        AUDIO_Mute = FALSE;
    }

    /* Updates register*/
    SET_FLAG_WRITE_CODEC_CRS( 6 );
}

/*******************************************************************************
*
*                            AUDIO_isMute
*
*******************************************************************************/
/**
*
*   Indicates if the audio is MUTE or not.
*   If MUT = ON, both buzzer, Loudspeaker and Headphones are cut off.
*
*   @return : true if AUDIO is Mute
*
**/
/******************************************************************************/
bool AUDIO_IsMute( void )
{
    return ( AUDIO_Mute == 1 );
}

/*******************************************************************************
*
*                            AUDIO_Inc_Volume
*
*******************************************************************************/
/**
*
*   Increment the volume of the loudspeaker and headphones.
*   @param[in] dB : number of decibels to increase volume
*
**/
/******************************************************************************/
void AUDIO_Inc_Volume( u8 dB )
{
    /* 2 dB step minimum*/
    if ( dB <= 1 ) dB = 2;

    /* Decrement the internal attenuation value*/
    AUDIO_Volume -= dB / 2;
    if ( AUDIO_Volume < AUDIO_MIN_ATTENUATION )
        AUDIO_Volume = AUDIO_MIN_ATTENUATION;

    /* Apply new volume to codec */
    AUDIO_Set_Volume( );
}

/*******************************************************************************
*
*                            AUDIO_Dec_Volume
*
*******************************************************************************/
/**
*
*   Decrement the volume of the loudspeaker and headphones.
*   @param[in] dB number of decibels to decrease volume
*
**/
/******************************************************************************/
void AUDIO_Dec_Volume( u8 dB )
{
    /* 2 dB step minimum*/
    if ( dB <= 1 ) dB = 2;

    /* Increment the internal attenuation value*/
    AUDIO_Volume += ( dB / 2 );
    if ( AUDIO_Volume > 15 )
        AUDIO_Volume = 15;

    /* Apply new volume to codec */
    AUDIO_Set_Volume( );
}

/*******************************************************************************
*
*                            AUDIO_ReadRegister
*
*******************************************************************************/
/**
*
*   Reads a data byte from one of STw5094A configuration registers.
*
*   @param[in] register_to_read : Specify the location where to read (the register number).
*                                   (0 to 21)
*   @return    unsigned byte    : The data read
*
*
**/
/******************************************************************************/
u8 AUDIO_ReadRegister( u8 register_to_read )
{
    u8 val = 0;
    if ( register_to_read < 22 )
    {
        /* Get the new register value*/
        AUDIO_I2C_ReadMultByte( register_to_read, 1, &val );
        /* Update the local register table*/
        AUDIO_CODEC_CRs[register_to_read] = val;
    }
    return val;
}

/*******************************************************************************
*
*                           AUDIO_WriteRegister
*
*******************************************************************************/
/**
*
*   Send a data byte to one of STw5094A configuration registers.
*
*   @param[in] register_to_write : specifies the location where to write (the register number).
*                                   (0 to 21)
*   @param[in] data_to_write     : the data that you want to write.
*   @return    None
*
*   @warning   The write process pass through an intermediary register table, so
*              the effective write to the audio codec will be delayed.
**/
/******************************************************************************/
void AUDIO_WriteRegister( u8 register_to_write, u8 data_to_write )
{
    if ( register_to_write < 22 )
    {
        /* Update the local register table*/
        AUDIO_CODEC_CRs[register_to_write] = data_to_write;
        /* Launch write register*/
        SET_FLAG_WRITE_CODEC_CRS( register_to_write );
    }
}

/*******************************************************************************
*
*                            AUDIO_SetLocalBufferSize
*
*******************************************************************************/
/**
*
*   Adjust the size of the local buffer used in MONO mode
*
*   @param[in] size     : size in bytes to assign to local buffer.
*   @return    None
*
**/
/******************************************************************************/
void AUDIO_SetLocalBufferSize( s32 size )
{

    /* Convert in 16 bits words size*/
    size /= 2;

    if ( size < 128 )
        size = 128;

    if ( size > LOCAL_BUFFER_SIZE )
        size = LOCAL_BUFFER_SIZE;

    Audio_buffer_local_size = size;
}
