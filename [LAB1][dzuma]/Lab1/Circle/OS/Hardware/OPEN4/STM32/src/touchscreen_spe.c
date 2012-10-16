/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     touchscreen_spe.c
* @brief    Touchscreen initialization, routines, init, handler
* @author   YRT
* @date     05/2010
* @note     Platform = Open4 STM32
*
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private define ------------------------------------------------------------*/
#define CROSS_SIZE     8    // Size of the cross used in calibration

/* Private variables ---------------------------------------------------------*/
divider_t Max_cnt_ts = 50;        // System tick divider

// Calibration informations
s32 TS_SeqCal = 0;
s32 fTS_InitCal = 0;
tPOINT TS_CalBuffer[3];

// Reference points in absolute screen coordinates
// NTRF: changed to reverse order for positive result values
const tPOINT TS_RefSample[3]    = { {216, 24}, {216, 230}, {24, 24}};

/* External variables ---------------------------------------------------------*/
extern u16 ADC_ConvertedValue[ADC_DMA_SIZE]; // DMA analog buffer
extern u16 TS_Sensibility;
extern tTOUCHSCR_Cal TS_CalibrationSetup;
extern tPOINT TS_ScreenSample[3];

/* Private functions prototypes ----------------------------------------------*/
void TOUCHSCREEN_Drawing( void );

/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
*
*                                TOUCHSCR_Handler
*
*******************************************************************************/
/**
*
*  Called by the CircleOS scheduler to manage the touchscreen.
*
*  @attention  This function must <b>NOT</b> be called by the user.
*
**/
/******************************************************************************/
void TOUCHSCR_Handler( void )
{
    static s32 divider_coord = 0;

    vs32 vX = 0;
    vs32 vY = 0;
    vs32 vT = 0;
    vs32 X, Y, T, q;

    tPOINT display, screen;

    // Don't execute, if it's not time
    if (( divider_coord++ % Max_cnt_ts ) != 0 )
        return;

    // Filtered values out of ADC_NB_SAMPLES samples
    vX = MedianFilter( ADC_ConvertedValue + OFS_CHANNEL_TS + 0 );
    vY = MedianFilter( ADC_ConvertedValue + OFS_CHANNEL_TS + 1 );
    vT = MedianFilter( ADC_ConvertedValue + OFS_CHANNEL_TS + 2 );

    // Conversion voltage => points
    q = ( 4096 - vY );
    X = ( 1000 *     vX ) / q;
    Y = ( 1000 * ( vY - vT ) ) / q;
    T = ( 1000 * ( vT - vX ) ) / q;

    // See if touch pressed
    TOUCHSCR_Info.TouchPress = ( vT < TS_Sensibility );
    if ( TOUCHSCR_Info.TouchPress )
    {
        TOUCHSCR_Info.xRaw = X;
        TOUCHSCR_Info.yRaw = Y;

        // Correction with calibration coefficients
        vX = (( TS_CalibrationSetup.A * X )
        + ( TS_CalibrationSetup.B * Y )
        + ( TS_CalibrationSetup.C ) )
        /   TS_CalibrationSetup.R;

        Y  = (( TS_CalibrationSetup.D * X )
        + ( TS_CalibrationSetup.E * Y )
        + ( TS_CalibrationSetup.F ) )
        /   TS_CalibrationSetup.R;

        X = vX;

        if ( X < 0 ) X = 0;
        if ( Y < 0 ) Y = 0;

        // Adapt coordinates versus screen position
        s32 x2 = 0, y2 = 0;
        switch ( LCD_GetScreenOrientation() )
        {
        case V12:
            x2 = X;
            y2 = Y;
            break;
        case V9:
            x2 = Screen_Height - Y;
            y2 = X;
            break;
        case V3:
            x2 = Y ;
            y2 = Screen_Width - X;
            break;
        case V6:
            x2 = ( Screen_Width - X );
            y2 = Screen_Height - Y;
            break;
        }

        // Update global structure for diffusion
        TOUCHSCR_Info.xAbsPos = X;
        TOUCHSCR_Info.yAbsPos = Y;
        TOUCHSCR_Info.xPos = x2 - LCD_Offset;
        TOUCHSCR_Info.yPos = y2 - LCD_Offset;

        // Reset stanby time-out
        POWER_Set_Time();
    }
    else
    {
        TOUCHSCR_Info.xRaw = -1;
        TOUCHSCR_Info.yRaw = -1;
        TOUCHSCR_Info.xAbsPos = -1;
        TOUCHSCR_Info.yAbsPos = -1;
        TOUCHSCR_Info.xPos = -1;
        TOUCHSCR_Info.yPos = -1;

    } // End if touchpress


    switch ( TOUCHSCR_Info.Mode )
    {
    case TS_CALIBRATION:
        TOUCHSCREEN_Calibration();
        break;
    case TS_DRAWING:
        if ( Y < Screen_Height )  // Discard buttons
            TOUCHSCREEN_Drawing();
        break;
    case TS_NORMAL:
    default:
        break;
    }

} // End of touchscreen handler


/*******************************************************************************
*
*                                TOUCHSCR_Init
*
*******************************************************************************/
/**
*
*  Called by the CircleOS scheduler to initialize the touchscreen.
*
*  @attention  This function must <b>NOT</b> be called by the user.
*
**/
/******************************************************************************/
void TOUCHSCR_Init( void )
{
    tPOINT samples[3];

    // Positioning initial values
    TOUCHSCR_Info.xAbsPos = -1;
    TOUCHSCR_Info.yAbsPos = -1;
    TOUCHSCR_Info.xPos = -1;
    TOUCHSCR_Info.yPos = -1;

    // Retrieve calibration informations from backup registers
    // saving source points instead of calibration matrix
    // to save backup registers
    samples[0].X = ( s32 )UTIL_ReadBackupRegister( BKP_TS_X0 );
    samples[0].Y = ( s32 )UTIL_ReadBackupRegister( BKP_TS_Y0 );
    samples[1].X = ( s32 )UTIL_ReadBackupRegister( BKP_TS_X1 );
    samples[1].Y = ( s32 )UTIL_ReadBackupRegister( BKP_TS_Y1 );
    samples[2].X = ( s32 )UTIL_ReadBackupRegister( BKP_TS_X2 );
    samples[2].Y = ( s32 )UTIL_ReadBackupRegister( BKP_TS_Y2 );

    // Test if first power up (backup registers empty)
    if ( samples[0].X == 0 )
    {
        // If yes, launch the calibration sequence
        TOUCHSCR_Info.Mode = TS_CALIBRATION;
    }
    else
    {
        // No, we can load calibration values
        TOUCHSCR_CalculateCalibration( samples );

        // Else TS operational
        TOUCHSCR_Info.Mode = TS_DRAWING;
    }
}

/*******************************************************************************
*
*                                TOUCHSCR calibration
*
*******************************************************************************/
enum MENU_code TOUCHSCREEN_Calibration( void )
{
    s32 xpos = TOUCHSCR_Info.xRaw;
    s32 ypos = TOUCHSCR_Info.yRaw;

    // Avoid time display
    fDisplayTime = FALSE;

    // Cancel calibration, if central button pushed
    if ( BUTTON_GetState() == BUTTON_PUSHED )
    {
        BUTTON_WaitForRelease();
        TS_SeqCal = 8;
        return;
    }

    // Force mode when the function is called from the menu
    if ( TOUCHSCR_Info.Mode != TS_CALIBRATION )
    {
        TOUCHSCR_Info.Mode = TS_CALIBRATION;
        TS_SeqCal = 0;
        fTS_InitCal = 0;
    }

    switch ( TS_SeqCal )
    {
    case 0 :
        if ( fTS_InitCal == 0 )
        {
            // Do it only once
            fTS_InitCal = 1;

            // Don't display main screen and block rotation during calibration
            POINTER_SetMode( POINTER_OFF );
            BUTTON_SetMode( BUTTON_ONOFF );
            LCD_SetScreenOrientation( V12 );
            LCD_SetRotateScreen( 0 );

            // Title
            LCD_FillRect_Circle( 0, 0, Screen_Width, Screen_Height, RGB_WHITE );
            DRAW_SetCharMagniCoeff( 2 );
            DRAW_DisplayStringWithMode( 0, 180, "TOUCHSCREEN", ALL_SCREEN, NORMAL_TEXT, CENTER );
            DRAW_DisplayStringWithMode( 0, 150, "CALIBRATION", ALL_SCREEN, NORMAL_TEXT, CENTER );
            DRAW_SetCharMagniCoeff( 1 );

            // Ask for the first button touch
            DRAW_DisplayStringWithMode( 0, 70, "Press on",  ALL_SCREEN, NORMAL_TEXT, CENTER );
            DRAW_DisplayStringWithMode( 0, 55, "the black cross",  ALL_SCREEN, NORMAL_TEXT, CENTER );
            DRAW_DisplayStringWithMode( 0, 40, "with a stylus",  ALL_SCREEN, NORMAL_TEXT, CENTER );
            DRAW_Cross_Absolute( TS_RefSample[0].X, TS_RefSample[0].Y, RGB_BLACK, CROSS_SIZE );
        }

        // When starting calibration from config menu with touchsreen
        // the first point was entered immediately.
        if ( !TOUCHSCR_Info.TouchPress )
        {
            TS_SeqCal++;
        }
        break;

    case 1:
        // Store first touch informations
        if ( TOUCHSCR_Info.TouchPress )
        {
            TS_CalBuffer[0].X = xpos;
            TS_CalBuffer[0].Y = ypos;
            TS_SeqCal++;
        }
        break;

    case 2 :
        // Ask for the second button touch
        if ( !( TOUCHSCR_Info.TouchPress ) )
        {
            LCD_FillRect_Circle( TS_RefSample[0].X - CROSS_SIZE, TS_RefSample[0].Y - CROSS_SIZE,
            ( CROSS_SIZE * 2 ) + 1, ( CROSS_SIZE * 2 ) + 1, RGB_WHITE );
            DRAW_Cross_Absolute( TS_RefSample[1].X, TS_RefSample[1].Y, RGB_BLACK, CROSS_SIZE );
            TS_SeqCal++;
        }
        break;

    case 3:
        // Store second touch informations
        if ( TOUCHSCR_Info.TouchPress )
        {
            TS_CalBuffer[1].X = xpos;
            TS_CalBuffer[1].Y = ypos;
            TS_SeqCal++;
        }
        break;

    case 4:
        if ( !( TOUCHSCR_Info.TouchPress ) )
        {
            // Ask for the third button touch
            LCD_FillRect_Circle( TS_RefSample[1].X - CROSS_SIZE, TS_RefSample[1].Y - CROSS_SIZE,
            ( CROSS_SIZE * 2 ) + 1, ( CROSS_SIZE * 2 ) + 1, RGB_WHITE );
            DRAW_Cross_Absolute( TS_RefSample[2].X, TS_RefSample[2].Y, RGB_BLACK, CROSS_SIZE );
            TS_SeqCal++;
        }
        break;

    case 5 :
        // Store third touch informations
        if ( TOUCHSCR_Info.TouchPress )
        {
            TS_CalBuffer[2].X = xpos;
            TS_CalBuffer[2].Y = ypos;

            TS_SeqCal++;
        }
        break;

    case 6 :
        // End of calibration
        if ( !( TOUCHSCR_Info.TouchPress ) )
        {
            // Calculate and update the coefficients
            TOUCHSCR_CalculateCalibration( TS_CalBuffer );

            // Save the calibration points
            UTIL_WriteBackupRegister( BKP_TS_X0, TS_CalBuffer[0].X );
            UTIL_WriteBackupRegister( BKP_TS_Y0, TS_CalBuffer[0].Y );
            UTIL_WriteBackupRegister( BKP_TS_X1, TS_CalBuffer[1].X );
            UTIL_WriteBackupRegister( BKP_TS_Y1, TS_CalBuffer[1].Y );
            UTIL_WriteBackupRegister( BKP_TS_X2, TS_CalBuffer[2].X );
            UTIL_WriteBackupRegister( BKP_TS_Y2, TS_CalBuffer[2].Y );

            LCD_FillRect_Circle( 0, 0, Screen_Width, Screen_Height, RGB_WHITE );
            DRAW_DisplayStringWithMode( 0, 55, "Calibration", ALL_SCREEN, NORMAL_TEXT, CENTER );
            DRAW_DisplayStringWithMode( 0, 40, "done.", ALL_SCREEN, NORMAL_TEXT, CENTER );
            DRAW_DisplayStringWithMode( 0, 15, "Touch the screen..", ALL_SCREEN, NORMAL_TEXT, CENTER );
            TS_SeqCal++;
        }
        break;

    case 7:
        if ( TOUCHSCR_Info.TouchPress )
            TS_SeqCal++;
        break;

    case 8:
    {
        // Restore normal conditions
        TS_SeqCal = 0;
        TOUCHSCR_Info.Mode = TS_NORMAL;

        // Can display main screen
        POINTER_SetMode( POINTER_ON );
        BUTTON_SetMode( BUTTON_ONOFF_FORMAIN );
        LCD_SetRotateScreen( 1 );
        DRAW_Clear();
    }
    break;

    } // switch ( TS_SeqCal )
} // end function

/// @endcond
