/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     circle_api.c
* @brief    The CircleOS API redirection table.
* @author   FL
* @date     07/2007
* @version  1.6
*
* @version  3.0 Add Primer2 management and ST lib v2.0.3
* @date     11/2008 
* @version  3.3 Add list management
* @date     12/2008 
* @version  4.0 Add Open4 Primer
* @date     10/2009 
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

#if SDCARD_AVAIL
#include "fs.h"
#endif

/* Private define ------------------------------------------------------------*/
typedef u32( *tCircleFunc0 )( void );
typedef u32( *tCircleFunc1 )( u32 param1 );
typedef u32( *tCircleFunc2 )( u32 param1, u32 param2 );
typedef u32( *tCircleFunc3 )( u32 param1, u32 param2, u32 param3 );
typedef u32( *tCircleFunc4 )( u32 param1, u32 param2, u32 param3, u32 param4 );
typedef u32( *tCircleFunc5 )( u32 param1, u32 param2, u32 param3, u32 param4, u32 param5 );
typedef u32( *tCircleFunc6 )( u32 param1, u32 param2, u32 param3, u32 param4, u32 param5, u32 param6 );

/* Variable ------------------------------------------------------------------*/
/*! CircleOS API redirection table. */
CONST_DATA tCircleFunc0 Circle_API[] =
{
    /* POINTER functions (0x00)*/
    ( tCircleFunc0 )POINTER_SetRect,
    ( tCircleFunc0 )POINTER_SetRectScreen,
    ( tCircleFunc0 )POINTER_GetCurrentAngleStart,
    ( tCircleFunc0 )POINTER_SetCurrentAngleStart,
    ( tCircleFunc0 )POINTER_GetCurrentSpeedOnAngle,
    ( tCircleFunc0 )POINTER_SetCurrentSpeedOnAngle,
    ( tCircleFunc0 )POINTER_SetMode,
    ( tCircleFunc0 )POINTER_GetMode,
    ( tCircleFunc0 )POINTER_SetCurrentPointer,
    ( tCircleFunc0 )POINTER_GetState,
    ( tCircleFunc0 )POINTER_Draw,
    ( tCircleFunc0 )POINTER_Save,
    ( tCircleFunc0 )POINTER_Restore,
    ( tCircleFunc0 )POINTER_GetPos,
    ( tCircleFunc0 )POINTER_SetPos,
    ( tCircleFunc0 )POINTER_SetApplication_Pointer_Mgr,
    ( tCircleFunc0 )POINTER_SetColor,
    ( tCircleFunc0 )POINTER_GetColor,
    ( tCircleFunc0 )POINTER_GetInfo,
    ( tCircleFunc0 )POINTER_SetCurrentAreaStore,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    /* DRAW functions (0x20)*/
    ( tCircleFunc0 )DRAW_SetDefaultColor,
    ( tCircleFunc0 )DRAW_Clear,
    ( tCircleFunc0 )DRAW_SetImage,
    ( tCircleFunc0 )DRAW_SetImageBW,
    ( tCircleFunc0 )DRAW_SetLogoBW,
    ( tCircleFunc0 )DRAW_DisplayVbat,
    ( tCircleFunc0 )DRAW_DisplayTime,
    ( tCircleFunc0 )DRAW_DisplayString,
    ( tCircleFunc0 )DRAW_DisplayStringInverted,
    ( tCircleFunc0 )DRAW_GetCharMagniCoeff,
    ( tCircleFunc0 )DRAW_SetCharMagniCoeff,
    ( tCircleFunc0 )DRAW_GetTextColor,
    ( tCircleFunc0 )DRAW_SetTextColor,
    ( tCircleFunc0 )DRAW_GetBGndColor,
    ( tCircleFunc0 )DRAW_SetBGndColor,
    ( tCircleFunc0 )DRAW_Line,
    ( tCircleFunc0 )DRAW_SetImageSel,
    ( tCircleFunc0 )DRAW_Putc,
    ( tCircleFunc0 )DRAW_Puts,
    ( tCircleFunc0 )DRAW_SetCursorPos,
    ( tCircleFunc0 )DRAW_GetCursorPos,
    ( tCircleFunc0 )DRAW_SetCursorMargin,
    ( tCircleFunc0 )DRAW_GetCursorMargin,
    ( tCircleFunc0 )DRAW_DisplayStringWithMode,
    ( tCircleFunc0 )DRAW_Circle,
    ( tCircleFunc0 )DRAW_Ellipse,
    ( tCircleFunc0 )DRAW_Polygon,
    0, 0, 0, 0, 0,

    /* LCD functions (0x40)*/
    ( tCircleFunc0 )LCD_SetRect_For_Cmd,
    ( tCircleFunc0 )LCD_GetPixel,
    ( tCircleFunc0 )LCD_DrawPixel,
    ( tCircleFunc0 )LCD_SendLCDCmd,
    ( tCircleFunc0 )LCD_SendLCDData,
    ( tCircleFunc0 )LCD_ReadLCDData,
    ( tCircleFunc0 )LCD_FillRect,
    ( tCircleFunc0 )LCD_DrawRect,
    ( tCircleFunc0 )LCD_DisplayChar,
    ( tCircleFunc0 )LCD_RectRead,
    ( tCircleFunc0 )LCD_SetBackLight,
    ( tCircleFunc0 )LCD_GetBackLight,
    ( tCircleFunc0 )LCD_SetRotateScreen,
    ( tCircleFunc0 )LCD_GetRotateScreen,
    ( tCircleFunc0 )LCD_SetScreenOrientation,
    ( tCircleFunc0 )LCD_GetScreenOrientation,
    ( tCircleFunc0 )LCD_SetBackLightOff,
    ( tCircleFunc0 )LCD_SetBackLightOn,
    ( tCircleFunc0 )LCD_SetFont,
    ( tCircleFunc0 )LCD_SetDefaultFont,
    ( tCircleFunc0 )LCD_SetOffset,
    ( tCircleFunc0 )LCD_GetScreenWidth,
    ( tCircleFunc0 )LCD_GetScreenHeight,
    ( tCircleFunc0 )LCD_SetFontDef,
    ( tCircleFunc0 )LCD_GetFontDef,
    ( tCircleFunc0 )LCD_ChangeFont,
    ( tCircleFunc0 )LCD_SetTransparency,
    ( tCircleFunc0 )LCD_GetTransparency,
    ( tCircleFunc0 )LCD_FullScreen,
    0, 0, 0,

    /* LED functions (0x60)*/
    ( tCircleFunc0 )LED_Set,

#if TOUCHSCREEN_AVAIL
    /* TOUCHSCREEN functions (0x61)*/
    ( tCircleFunc0 )TOUCHSCR_GetPos,
    ( tCircleFunc0 )TOUCHSCR_GetAbsPos,
    ( tCircleFunc0 )TOUCHSCR_IsPressed,
    ( tCircleFunc0 )TOUCHSCR_GetMode,
    ( tCircleFunc0 )TOUCHSCR_SetSensibility,
    ( tCircleFunc0 )LIST_Manager,
    ( tCircleFunc0 )LIST_Set,
    ( tCircleFunc0 )TOUCHSCR_GetPosX,
    ( tCircleFunc0 )TOUCHSCR_GetPosY,

    /* TOOLBAR functions (0x6A)*/
    ( tCircleFunc0 )TOOLBAR_Set,
    ( tCircleFunc0 )TOOLBAR_SetDefaultToolbar,
    ( tCircleFunc0 )TOOLBAR_ChangeButton,
    0, 0, 0,
#else
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#endif

    /* MEMS functions (0x70)*/
    ( tCircleFunc0 )MEMS_GetPosition,
    ( tCircleFunc0 )MEMS_GetRotation,
    ( tCircleFunc0 )MEMS_SetNeutral,
    ( tCircleFunc0 )MEMS_GetInfo,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    /* BUTTON functions (0x80)*/
    ( tCircleFunc0 )BUTTON_GetState,
    ( tCircleFunc0 )BUTTON_SetMode,
    ( tCircleFunc0 )BUTTON_GetMode,
    ( tCircleFunc0 )BUTTON_WaitForRelease,
#if JOYSTICK_AVAIL
    /* JOYSTICK functions (0x84)*/
    ( tCircleFunc0 )JOYSTICK_GetState,
    ( tCircleFunc0 )JOYSTICK_WaitForRelease,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#else
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#endif

    /* BUZZER functions (0x90)*/
    ( tCircleFunc0 )BUZZER_SetMode,
    ( tCircleFunc0 )BUZZER_GetMode,
    ( tCircleFunc0 )BUZZER_PlayMusic,
#if AUDIO_AVAIL
    /* AUDIO functions (0x93)*/
    ( tCircleFunc0 )AUDIO_SetMode,
    ( tCircleFunc0 )AUDIO_GetMode,
    ( tCircleFunc0 )AUDIO_Play,
    ( tCircleFunc0 )AUDIO_SPEAKER_OnOff,
    ( tCircleFunc0 )AUDIO_MUTE_OnOff,
    ( tCircleFunc0 )AUDIO_IsMute,
    ( tCircleFunc0 )AUDIO_Inc_Volume,
    ( tCircleFunc0 )AUDIO_Dec_Volume,
    ( tCircleFunc0 )AUDIO_Playback_GetStatus,
    ( tCircleFunc0 )AUDIO_Recording_GetStatus,
    ( tCircleFunc0 )AUDIO_Record,
    ( tCircleFunc0 )AUDIO_ReadRegister,
    ( tCircleFunc0 )AUDIO_WriteRegister,
#else
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#endif

    /*MENU functions (0xA0)*/
    ( tCircleFunc0 )MENU_Set,
    ( tCircleFunc0 )MENU_Remove,
    ( tCircleFunc0 )MENU_Question,
    ( tCircleFunc0 )MENU_Print,
    ( tCircleFunc0 )MENU_ClearCurrentCommand,
    ( tCircleFunc0 )MENU_SetLevelTitle,
    ( tCircleFunc0 )MENU_SetTextColor,
    ( tCircleFunc0 )MENU_GetTextColor,
    ( tCircleFunc0 )MENU_SetBGndColor,
    ( tCircleFunc0 )MENU_GetBGndColor,
    ( tCircleFunc0 )MENU_Quit,
    ( tCircleFunc0 )MENU_SetLevel_Ini,
    ( tCircleFunc0 )MENU_ClearCurrentMenu,
    ( tCircleFunc0 )MENU_SetLevel_Mgr,
    ( tCircleFunc0 )MENU_SetAppliDivider,
    ( tCircleFunc0 )MENU_RestoreAppliDivider,

    /* UTIL functions (0xB0)*/
    ( tCircleFunc0 )UTIL_SetPll,
    ( tCircleFunc0 )UTIL_GetPll,
    ( tCircleFunc0 )UTIL_uint2str,
    ( tCircleFunc0 )UTIL_int2str,
    ( tCircleFunc0 )UTIL_GetVersion,
    ( tCircleFunc0 )UTIL_ReadBackupRegister,
    ( tCircleFunc0 )UTIL_WriteBackupRegister,
    ( tCircleFunc0 )UTIL_GetBat,
    ( tCircleFunc0 )UTIL_GetUsb,
    ( tCircleFunc0 )UTIL_SetIrqHandler,
    ( tCircleFunc0 )UTIL_GetIrqHandler,
    ( tCircleFunc0 )UTIL_SetSchHandler,
    ( tCircleFunc0 )UTIL_GetSchHandler,
    ( tCircleFunc0 )UTIL_GetTemp,
    ( tCircleFunc0 )UTIL_SetTempMode,
    ( tCircleFunc0 )UTIL_GetPrimerType,

    /* RTC functions (0xC0)*/
    ( tCircleFunc0 )RTC_SetTime,
    ( tCircleFunc0 )RTC_GetTime,
    ( tCircleFunc0 )RTC_DisplayTime,
#if AUDIO_AVAIL
    /* More audio Functions (0xC3), using buffers*/
    ( tCircleFunc0 )AUDIO_SetLocalBufferSize,
    ( tCircleFunc0 )AUDIO_RecordBuffer_GetStatus,
    ( tCircleFunc0 )AUDIO_Record_Stop,
    ( tCircleFunc0 )AUDIO_PlaybackBuffer_GetStatus,
    ( tCircleFunc0 )AUDIO_Playback_Stop,
#else
    0, 0, 0, 0, 0,
#endif

    /* UTIL functions (0xC8)*/
    ( tCircleFunc0 )UTIL_GetAppAddress,
    ( tCircleFunc0 )UTIL_IsStandAloneMode,
    0, 0, 0, 0, 0, 0,

    /* PWR functions (0xD0)*/
    ( tCircleFunc0 )SHUTDOWN_Action,
    0, 0, 0, 0, 0, 0, 0,

#if SDCARD_AVAIL
    /* Filesystem (0xD8)*/
    ( tCircleFunc0 )FS_Mount,
    ( tCircleFunc0 )FS_Unmount,
    ( tCircleFunc0 )FS_OpenFile,
    ( tCircleFunc0 )FS_ReadFile,
    ( tCircleFunc0 )FS_WriteFile,
    ( tCircleFunc0 )FS_Close,
    ( tCircleFunc0 )FS_Seek,
    ( tCircleFunc0 )FS_Delete,
    ( tCircleFunc0 )FS_GetNextEntry,
    ( tCircleFunc0 )FS_OpenDirectory,
    ( tCircleFunc0 )FS_GetVolumeInfo,
    ( tCircleFunc0 )FS_Explorer_Ini,
    ( tCircleFunc0 )FS_Explorer,
    ( tCircleFunc0 )FS_GetSDCardCurrentPath,
    ( tCircleFunc0 )FS_GetSDCardVolInfo,
    ( tCircleFunc0 )FS_GetPathFilter,
    ( tCircleFunc0 )FS_SetPathFilter,
#endif
};


