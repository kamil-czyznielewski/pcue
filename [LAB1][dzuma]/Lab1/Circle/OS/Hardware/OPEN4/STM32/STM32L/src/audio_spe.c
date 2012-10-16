/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     audio_spe.c
* @brief    Small audio management (Mute and volume only).
* @author   YRT
* @date     08/2010
* @version  4.0 Platform = Open4 STM32L without codec
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
s8   AUDIO_Volume                         = 0;           // number of 2 dB steps for attenuation volume
bool AUDIO_SpeakerOn                      = 1;           // loudspeaker active or not
bool AUDIO_SpeakerOnOld                   = 0;           // flag for change detection
bool AUDIO_Mute                           = 0;           // sound (loudspeaker+headphones) active or not

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/* External variables --------------------------------------------------------*/
extern u16 waveForm[];

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
    /* Set the default values*/
    if ( fFirstStartup == FALSE )
    {
        /* Restore the configuration    */
        AUDIO_Volume = (( UTIL_ReadBackupRegister( BKP_SYS2 ) ) & BKPMASK_S2_VOLUME ) >> 8 ;
        AUDIO_Mute = (( UTIL_ReadBackupRegister( BKP_SYS2 ) ) & BKPMASK_S2_MUTE ) ? 1 : 0;
        AUDIO_BuzzerOn = (( UTIL_ReadBackupRegister( BKP_SYS2 ) ) & BKPMASK_S2_BUZZER ) ? 1 : 0;

    }
    else
    {
        AUDIO_Mute = FALSE;
        AUDIO_Volume = AUDIO_MIN_ATTENUATION;
        AUDIO_BuzzerOn = TRUE;
    }

    AUDIO_Set_Volume();

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
    if (AUDIO_Mute)
        waveForm[0] = 0;
    else
        // DAC with 4096 points
        waveForm[0] = 0x0FFF - ( AUDIO_Volume * 100 );
}


/// @endcond

/* Public functions ----------------------------------------------------------*/

/*******************************************************************************
*
*                            AUDIO_MUTE_OnOff
*
*******************************************************************************/
/**
*
*   Switch the sound ON or OFF.
*
*   @param[in] mode : ON or OFF
*
**/
/******************************************************************************/
void AUDIO_MUTE_OnOff( ON_OFF_enum mode )
{
    if ( mode != OFF )
    {
        AUDIO_Mute = TRUE;
        waveForm[0] = 0;
    }
    else
    {
        AUDIO_Mute = FALSE;
        AUDIO_Set_Volume();
    }
}

/*******************************************************************************
*
*                            AUDIO_isMute
*
*******************************************************************************/
/**
*
*   Indicates if the audio is MUTE or not.
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
*   Increment the volume of the loudspeaker.
*   @param[in] dB : number of decibels to increase volume
*
**/
/******************************************************************************/
void AUDIO_Inc_Volume( u8 dB )
{
    /* Decrement the internal attenuation value*/
    AUDIO_Volume -= dB;
    if ( AUDIO_Volume < AUDIO_MIN_ATTENUATION )
        AUDIO_Volume = AUDIO_MIN_ATTENUATION;

    /* Apply new volume */
    AUDIO_Set_Volume( );
}

/*******************************************************************************
*
*                            AUDIO_Dec_Volume
*
*******************************************************************************/
/**
*
*   Decrement the volume of the loudspeaker.
*   @param[in] dB number of step to decrease volume
*
**/
/******************************************************************************/
void AUDIO_Dec_Volume( u8 dB )
{
    /* Increment the internal attenuation value*/
    AUDIO_Volume +=  dB;
    if ( AUDIO_Volume > AUDIO_MAX_ATTENUATION )
        AUDIO_Volume = AUDIO_MAX_ATTENUATION;

    /* Apply new volume */
    AUDIO_Set_Volume( );
}

