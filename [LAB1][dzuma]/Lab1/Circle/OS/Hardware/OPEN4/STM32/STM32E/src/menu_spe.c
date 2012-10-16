/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     menu_spe.c
* @brief    Various utilities for menus, depending on primer version
* @author   EL
* @date     07/2009
* @Note     Platform = Open4 STM32E Daughter Board
**/
/******************************************************************************/

/* Includes -------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private Defines--------------------------------------------------------------*/

/* Private function prototypes ------------------------------------------------*/
enum MENU_code fAboutIni( void );
enum MENU_code fAboutMgr( void );
enum MENU_code fCalibration( void );
enum MENU_code fConfig( void );
enum MENU_code fInBoth( void );
enum MENU_code fInJoystick( void );
enum MENU_code fInMems( void );
enum MENU_code fInTchscr( void );
enum MENU_code fInterface( void );
enum MENU_code fPower( void );
enum MENU_code fQuit( void );
enum MENU_code fSDCard( void );
enum MENU_code fSetAutorun( void );
enum MENU_code fSetBacklight_Mgr( void );
enum MENU_code fSetBeep( void );
enum MENU_code fSetInput( void );
enum MENU_code fSetMenuFont_Ini( void );
enum MENU_code fSetMenuFont_Mgr( void );
enum MENU_code fSetPllRange_Mgr( void );
enum MENU_code fSetSpeaker( void );
enum MENU_code fSetTime_Ini( void );
enum MENU_code fSetTime_Mgr( void );
enum MENU_code fShutdown( void );
enum MENU_code fTest_Mgr( void );

//enum MENU_code Explorer_Handler( void );
enum MENU_code FctAppIni( void );
enum MENU_code FctAppMgr( void );

void DB_LED_Init( void );


/* Private variables ---------------------------------------------------------*/
u8     TextBuffer[10];
s32    RecordPlaySeq = -1;
bool   AUDIO_BuzzerOn = 1;              // Buzzer active or not
bool   AutorunOn = 0;                   // Application autorun or not
bool   MenuFirstDisplay = 1;

/* Extern variables ----------------------------------------------------------*/
extern divider_t menu_divider_coord;
extern uint_t Current_CCR_BackLightStart;
extern Rotate_H12_V_Match_TypeDef   previous_Screen_Orientation;
extern tMenuItem *(( *ApplicationTable )[] );


tMenu MainMenu =
{
    1,
    "Main Menu",
    7, 0, 0, 0, 0, 0,
    0,
    {
        { "Config  ",  fConfig,    0,  0 },
        { " ",         fQuit,      0,  1 },
        { "Applic. ",  FctAppIni,  FctAppMgr,  0 },
        { "SDCard",    fSDCard,    0,  0 },
        { "About ",    fAboutIni,  fAboutMgr,  0 },
        { "Shutdown",  fShutdown,  0,  1 },
        { "Quit",      fQuit,      0,  1 }
    }
};

tMenu SDcardMenu =
{
    1,
    "SDCard",
    3, 0, 0, 0, 0, 0,
    0,
    {
        { "Local",      FS_Explorer_Ini,   FS_Explorer_Handler,   0 },
        { "PC",         Fct_Download_Ini,  Fct_Download_Handler,  0 },
        { "Quit",       fQuit,      0,  1 }
    }
};

tMenu ConfigMenu =
{
    1,
    "Configuration",
    6, 0, 0, 0, 0, 0,
    0,
    {
        { "Power",         fPower,             0, 0 },
        { "Interface",     fInterface,         0, 0 },
        { "Autorun",       fSetAutorun,        0, 0 },
        { "Time",          fSetTime_Ini,       fSetTime_Mgr,      0 },
        { "Test",          MENU_SetLevel_Ini,  fTest_Mgr,         0 },
        { "Quit",          fQuit,              0,                 1 }
    }
};

tMenu PowerMenu =
{
    1,
    "Power",
    3, 0, 0, 0, 0, 0,
    0,
    {
        { "CPU Freq",    MENU_SetLevel_Ini, fSetPllRange_Mgr,  0 },
        { "Backlight",   MENU_SetLevel_Ini, fSetBacklight_Mgr, 0 },
        { "Quit",        fQuit,             0,                 1 }
    }
};

tMenu InterfaceMenu =
{
    1,
    "Interface",
    6, 0, 0, 0, 0, 0,
    0,
    {
        { "User Input",    fSetInput,         0,                 0 },
        { "Loudspeaker",   fSetSpeaker,       0,                 0 },
        { "Menu",          fSetMenuFont_Ini,  fSetMenuFont_Mgr,  0 },
        { "Beep",          fSetBeep,          0,                 0 },
        { "TS Calibrate",  fCalibration,      TOUCHSCREEN_Calibration,                 0 },
        { "Quit",          fQuit,             0,                 1 }
    }
};

tMenu InputMenu =
{
    1,
    "User Input",
    5, 0, 0, 0, 0, 0,
    0,
    {
        { "MEMS",           fInMems,      0,                 1 },
        { "JOYSTICK",       fInJoystick,  0,                 1 },
        { "BOTH (M+J)",     fInBoth,      0,                 1 },
        { "TCHSCR",         fInTchscr,    0,                 1 },
        { "Quit",           fQuit,        0,                 1 }
    }
};

tList FontList =
{
    1,
    "Menu Fonts",
    3, 
    0, 0, 0, 0, 0,
    3,
    0,
    0,
} ;

const uint_t CCR_BackLight_Tab[5]  = {0x4000, 0x6000, 0x8000, 0xA000, 0xC000};

/*******************************************************************************
*
*                                MENU_Init
*
*******************************************************************************/
/**
*
*  General initialization MENU handler.
*
*  @attention  This function must <b>NOT</b> be called by the user.
*
**/
/******************************************************************************/
void MENU_Init( void )
{
    /* Restore the interface configuration*/
    if ( fFirstStartup == FALSE )
    {
        Menu_Font = ( UTIL_ReadBackupRegister( BKP_SYS4 ) ) & BKPMASK_S4_FONT;
    }
    /* Or put the default configuration */
    else
    {
        Menu_Font = 0;
    }
}

// menu.c========================================================================

/*******************************************************************************
*
*                                fAboutIni
*
*******************************************************************************/
/**
*  Initialization function for the 'About' application.
*
*  @retval  MENU_CONTINUE_COMMAND.
*
**/
/******************************************************************************/
enum MENU_code fAboutIni( void )
{
    MenuFirstDisplay = 1;

    // Set CircleOS into the wanted state.
    MENU_Remove();

    POINTER_SetMode( POINTER_OFF );
    BUTTON_SetMode( BUTTON_ONOFF );

    return MENU_CONTINUE_COMMAND;
}

/*******************************************************************************
*
*                                fAboutMgr
*
*******************************************************************************/
/**
*  'Main' function for the 'About' application.
*
*  @retval  MENU_CONTINUE  Keep on calling me.
*  @retval  MENU_LEAVE     Application is over!
*
**/
/******************************************************************************/
enum MENU_code fAboutMgr( void )
{
    int            LastApplication   = -1;
    int            AppSizeCounter    = 0;
    int            freeMemoryFlash   = 0;
    int            i;
    long unsigned  addr;
    unsigned long* lptr              = ( unsigned long* ) CIRCLEOS_FAT_ADDRESS; // YRT20090114
    tMenuItem *(( *ApplicationTable )[] );

    ApplicationTable  = ( tMenuItem * ((* )[] ) )( *lptr );

    MENU_ClearCurrentMenu();

    for ( i = 0 ; i < MAXAPP ; i++ )
    {
        if (( *ApplicationTable )[ -i ] == APP_VOID ) //has been programmed
        {
            break;
        }

        LastApplication =  i;
    }

    if ( LastApplication != -1 )
    {
        for ( i = 0; i <= LastApplication; i++ )
        {
            addr   = ( long unsigned )( *ApplicationTable )[ -i ] ;
            addr &= 0xFF000000;

            AppSizeCounter += ( addr >> 24 );
        }
    }

    // When the user pushedthe button, it's time to leave!
    if ( BUTTON_GetState() == BUTTON_PUSHED )
    {
        return fQuit();
    }

    // If screen orientation has changed, we redraw the screen!
    if ( MenuFirstDisplay == 0 )
    {
        if ( LCD_GetScreenOrientation() != previous_Screen_Orientation )
        {
            MenuFirstDisplay = 1;
        }
    }

    // Let's draw the 'About' screen.
    if ( MenuFirstDisplay )
    {
        DRAW_SetCharMagniCoeff( 2 );

        previous_Screen_Orientation   = LCD_GetScreenOrientation();

        DRAW_SetTextColor( RGB_BLACK );
        freeMemoryFlash               = 512 - 180 - AppSizeCounter;
        DRAW_DisplayStringWithMode( 0, 200, "STM32E Primer", ALL_SCREEN, NORMAL_TEXT, CENTER );
        DRAW_DisplayStringWithMode( 0, 174, "CPU:STM32F103VET6", ALL_SCREEN, NORMAL_TEXT, CENTER );
        DRAW_SetCharMagniCoeff( 1 );
        DRAW_DisplayString( 62,   148,  "OS : V4.0    24 KB",   18 );
        DRAW_DisplayString( 62,   122,  "App: 2       16 KB",   18 );
        DRAW_DisplayString( 62,    96,  "Avail FLASH: 88 KB",   18 );

        DRAW_SetTextColor( RGB_BLUE );
        DRAW_DisplayString( 97,    148,  UTIL_GetVersion(),      6 );
        DRAW_DisplayString( 146,    148,  "180",                 3 );

        UTIL_uint2str( TextBuffer, LastApplication + 1, 2, 0 );
        DRAW_DisplayString( 97,    122,  TextBuffer,             2 );
        UTIL_uint2str( TextBuffer, AppSizeCounter, 3, 0 );
        DRAW_DisplayString( 146,    122,  TextBuffer,             3 );

        UTIL_uint2str( TextBuffer, freeMemoryFlash , 3, 0 );
        DRAW_DisplayString( 146,    96,  TextBuffer,             3 );

        DRAW_SetCharMagniCoeff( 2 );
        DRAW_SetTextColor( RGB_MAKE( 0, 183, 239 ) );
        DRAW_SetBGndColor( RGB_BLACK );
        LCD_FillRect_Circle( 0, 0, Screen_Width, Char_Height * 2, RGB_BLACK );
        DRAW_DisplayStringWithMode( 0, 0, "STM32circle.com", ALL_SCREEN, NORMAL_TEXT, CENTER );

        DRAW_SetDefaultColor();

        DRAW_SetCharMagniCoeff( 1 );
        
        MenuFirstDisplay = 0;
    }

return MENU_CONTINUE;
}



// menu_settings.c========================================================================

/*******************************************************************************
*
*                                fInMems
*
*******************************************************************************/
/**
*  Select the MEMS as user input
*
*  @retval  MENU_LEAVE
*
**/
/******************************************************************************/
enum MENU_code fInMems( void )
{
    JoystickAsInput = 0;
    MemsAsInput = 1;
    TchscrAsInput = 0;

    return fQuit();
}

/*******************************************************************************
*
*                                fInJoystick
*
*******************************************************************************/
/**
*  Select the JOYSTICK as user input
*
*  @retval  MENU_LEAVE
*
**/
/******************************************************************************/
enum MENU_code fInJoystick( void )
{
    JoystickAsInput = 1;
    MemsAsInput = 0;
    TchscrAsInput = 0;

    return fQuit();
}

/*******************************************************************************
*
*                                fInBoth
*
*******************************************************************************/
/**
*  Select both the MEMS and the JOYSTICK as user input
*
*  @retval  MENU_LEAVE
*
**/
/******************************************************************************/
enum MENU_code fInBoth( void )
{
    JoystickAsInput = 1;
    MemsAsInput = 1;
    TchscrAsInput = 0;

    return fQuit();
}

/*******************************************************************************
*
*                                fInTchscr
*
*******************************************************************************/
/**
*  Select TOUCHSCREEN  as input.
*
*  @retval  MENU_LEAVE
*
**/
/******************************************************************************/
enum MENU_code fInTchscr( void )
{
    JoystickAsInput = 0;
    MemsAsInput = 0;
    TchscrAsInput = 1;

    return fQuit();
}

/*******************************************************************************
*
*                                fCalibration
*
*******************************************************************************/
/**
*  Launch a touchscreen calibration
*
*  @retval  MENU_LEAVE_AS_IT
*
**/
/******************************************************************************/
enum MENU_code fCalibration( void )
{
    extern s32 TS_SeqCal, fTS_InitCal;

    TS_SeqCal = 0;
    fTS_InitCal = 0;
    TOUCHSCREEN_Calibration();
    return MENU_LEAVE_AS_IT;
}

/*******************************************************************************
*
*                                fTest_Mgr
*
*******************************************************************************/
/**
*  Hardware test application.
*
*  @retval  MENU_CONTINUE  Keep on calling me.
*  @retval  MENU_LEAVE     Application is over.
*
**/
/******************************************************************************/
enum MENU_code fTest_Mgr( void )
{
    int fRedrawn = 0;
    static bool fFirstTime = 1;
    static s8 AUDIO_SpeakerOn_bak;
    static s8 AUDIO_Volume_bak;

    if ( fFirstTime )
    {
        AUDIO_SpeakerOn_bak = AUDIO_SpeakerOn;
        AUDIO_Volume_bak = AUDIO_Volume;
        AUDIO_Volume = AUDIO_MIN_ATTENUATION;
        AUDIO_SpeakerOn = 1;
        UTIL_SetPll( SPEED_VERY_HIGH );
        fFirstTime = 0;
        MenuFirstDisplay = 1;
    }

    // Hide menu.
    CurrentMenu = 0;
    fDisplayTime = 0; // force display time - battery status

    // Character magnify.
    DRAW_SetCharMagniCoeff( 2 );

    // If screen orientation has changed, redraw screen.
    if ( MenuFirstDisplay == 0 )
    {
        if ( LCD_GetScreenOrientation() != previous_Screen_Orientation )
        {
            MenuFirstDisplay = 1;
        }
    }

    if ( MenuFirstDisplay )
    {
        previous_Screen_Orientation = LCD_GetScreenOrientation();
        POINTER_SetMode( POINTER_OFF );
        LCD_SetRotateScreen( 0 );
        LCD_SetScreenOrientation( V12 );


        // Red LED blinks at high frequency.
        LED_Set( LED_RED, LED_BLINKING_HF );

        // Green LED blinks at low frequency.
        LED_Set( LED_GREEN, LED_BLINKING_LF );

        // Init daughter board LED
        DB_LED_Init();

        // Draw a little message on the LCD screen.
        DRAW_SetDefaultColor();
        DRAW_DisplayStringWithMode( 0, Screen_Height - 2 * Char_Height, "Hardware Test", ALL_SCREEN, INVERTED_TEXT, CENTER );
        MenuFirstDisplay = 0;

        // Drawn that rectangle on the screen.
        LCD_FillRect_Circle( BAR_LEFT + 1, BAR_BOTTOM + 1, MAXLEVEL * WIDTH_SQUARE  , WIDTH_SQUARE - 2, RGB_YELLOW );
        LCD_DrawRect( BAR_LEFT, BAR_BOTTOM, MAXLEVEL * WIDTH_SQUARE + 2 , WIDTH_SQUARE, RGB_BLUE );

        // Restrict the move of the pointer to a rectangle.
        POINTER_SetRect( BAR_LEFT + 1, BAR_BOTTOM + 1, MAXLEVEL * WIDTH_SQUARE - POINTER_WIDTH - 2, WIDTH_SQUARE - POINTER_WIDTH - 2 );
        POINTER_SetMode( POINTER_ON );


        fRedrawn = 1;
        RecordPlaySeq = -1;
        AUDIO_Welcome_Msg();
    } // end FirstDisplay

    // The button is pushed, let's get out of here!
    if ( BUTTON_GetState() == BUTTON_PUSHED )
    {
        int i;

        BUTTON_WaitForRelease();

        // Reset CircleOS state.
        MENU_ClearCurrentCommand();
        DRAW_SetDefaultColor();
        DRAW_Clear();
        POINTER_SetMode( POINTER_ON );
        BUTTON_SetMode( BUTTON_ONOFF_FORMAIN );
        POINTER_SetRectScreen();
        DRAW_SetCharMagniCoeff( 1 );

        // Reset backlight PWM rate to its default value.
        LCD_SetBackLight( 0 );

        // Switch LEDs off.
        LED_Set( LED_RED, LED_OFF );
        LED_Set( LED_GREEN, LED_OFF );
        GPIO_WriteBit( GPIOx_DB_LED, GPIO_Pin_DB_LED1, Bit_SET );
        GPIO_WriteBit( GPIOx_DB_LED, GPIO_Pin_DB_LED2, Bit_SET );

        // No sound
        AUDIO_Playback_status = NO_SOUND;
        AUDIO_Recording_status = NO_RECORD;

        // Restore default mode
        AUDIO_SetMode( AUDIO_MODE, LG_8_BITS, FRQ_16KHZ, MONO );

        // Restore the previous config
        AUDIO_SpeakerOn = AUDIO_SpeakerOn_bak;
        AUDIO_Volume = AUDIO_Volume_bak;
        fFirstTime  = 1;

        // Search if test aplication present in flash
        for ( i = 0; i < MAXAPP; i++ )
        {
            extern tMenuItem*(( *ApplicationTable )[] );
            tMenuItem*  curapp;
            unsigned addr;

            if (( *ApplicationTable )[ -i ] == APP_VOID ) //has been programmed
            {
                break;
            }
            else
            {
                addr   = ( long unsigned )( *ApplicationTable )[ -i ] ;
                addr &= 0x00FFFFFF;
                addr |= 0x08000000;
                curapp = ( tMenuItem* ) addr;
                if ( !strcmp( curapp->Text, "COMMTEST" ) )
                {
                    LCD_SetOffset( OFFSET_ON );
                    CurrentCommand = curapp;
                    CurrentMenu = 0;
                    BUTTON_SetMode( BUTTON_ONOFF );
                    return CurrentCommand ->Fct_Init();
                }
            }
        }
        LCD_SetRotateScreen( 1 );
        LCD_SetScreenOrientation( previous_Screen_Orientation );


        return MENU_LEAVE;
    }

    // Increment the backlight PWM.
    Current_CCR_BackLightStart += 0x800;

    //-- Parrot mode in order to test microphone and speaker
    if (( AUDIO_Playback_status == NO_SOUND ) && ( AUDIO_Recording_status == NO_RECORD ) )
    {

        // Values -1 and 2 for RecordPlaySeq correspond to the first loop.
        // Recording requires a delay (internal to the Codec?) before
        // getting non-zero values. The first cycle provides this delay

        if (( RecordPlaySeq == 0 ) || ( RecordPlaySeq == -1 ) )
        {
            // End of playback, launch the record
            if ( AUDIO_Playback_status == NO_SOUND )
            {
                AUDIO_SetMode( VOICE_MODE, LG_16_BITS, FRQ_16KHZ, MONO );
                // Take all the available memory for the record (= RAM_BASE - data size - 500 bytes for the stack)
                AUDIO_Record(( sound_type* ) RAM_BASE , (( CIRCLEOS_RAM_OFS ) / sizeof( voice_type ) ) - 500 ) ;
            }
            if ( RecordPlaySeq != -1 )
            {
                // Red LED blinks at high frequency.
                LED_Set( LED_GREEN, LED_OFF );
                LED_Set( LED_RED, LED_ON );
                DRAW_DisplayStringWithMode( 0, 160, "** RECORDING **", ALL_SCREEN, NORMAL_TEXT, CENTER );
                RecordPlaySeq = 1;
            }
            else
                RecordPlaySeq = 2;

        }
        else
        {
            if ( AUDIO_Recording_status == NO_RECORD )
            {
                // End of recording, launch the playback
                AUDIO_SetMode( AUDIO_MODE, LG_16_BITS, FRQ_16KHZ, MONO );
                AUDIO_Play(( sound_type* ) RAM_BASE , (( CIRCLEOS_RAM_OFS ) / sizeof( voice_type ) ) - 500 ) ;
            }
            if ( RecordPlaySeq != 2 )
            {
                // Red LED blinks at high frequency.
                LED_Set( LED_GREEN, LED_ON );
                LED_Set( LED_RED, LED_OFF );
                DRAW_DisplayStringWithMode( 0, 160, "** PLAYING **", ALL_SCREEN, NORMAL_TEXT, CENTER );
            }
            RecordPlaySeq = 0;
        }
    }

    if (( AUDIO_Recording_status == IS_RECORDING )  && (( AUDIO_RecordBuffer_GetStatus( 0 ) & HIGH_FULL ) ) )
        AUDIO_Record_Stop();


    // If backlight PWM is too high, restart with a lower value.
    if ( Current_CCR_BackLightStart > 0xF000 )
    {
        Current_CCR_BackLightStart = 0x2000;
    }

    // Every 59th calls display battery voltage and replay melody sample.
    // Note: due to the fRedrawn == 0, the first time the % 59 == 0
    //       (when divider_coord is null), the code block isn't executed.
    if ((( menu_divider_coord++ % 37 ) == 0 ) && ( fRedrawn == 0 ) )
    {
        char TextBuffer[5];
        *TextBuffer = '%'; *( TextBuffer + 1 ) = 0; // YRT20080204 : to avoid memcpy

        DRAW_DisplayVbat( 20, 12 );
        UTIL_uint2str( TextBuffer + 1, PWR_BatteryLevel, 3, 0 );
        DRAW_DisplayString( 170, 12, TextBuffer, 4 );
    }

    // Toggle daughter card leds
    if (( menu_divider_coord % 2 ) == 0 )
    {
        GPIO_WriteBit( GPIOx_DB_LED, GPIO_Pin_DB_LED1, Bit_SET );
        GPIO_WriteBit( GPIOx_DB_LED, GPIO_Pin_DB_LED2, Bit_RESET );
    }
    else
    {
        GPIO_WriteBit( GPIOx_DB_LED, GPIO_Pin_DB_LED1, Bit_RESET );
        GPIO_WriteBit( GPIOx_DB_LED, GPIO_Pin_DB_LED2, Bit_SET );
    }

    return MENU_CONTINUE;
}

/*******************************************************************************
*
*                                fSetInput
*
*******************************************************************************/
/**
*  Set if the commands are done by the joystick instead of the mems
*
*  @return  MENU_CHANGE
*
**/
/******************************************************************************/
enum MENU_code fSetInput( void )
{
    MENU_Set( &InputMenu );

    return MENU_CHANGE;
}

/*******************************************************************************
*
*                                fSetSpeaker
*
*******************************************************************************/
/**
*  Set if the loudspeaker is active or not
*
*  @return  MENU_CHANGE
*
**/
/******************************************************************************/
enum MENU_code fSetSpeaker( void )
{
    MENU_Question( "LoudSpeaker On ?", &AUDIO_SpeakerOn );
    return MENU_CHANGE;
}

/*******************************************************************************
*
*                                fSetBeep
*
*******************************************************************************/
/**
*  Set if the Beep is active or not
*
*  @return  MENU_CHANGE
*
**/
/******************************************************************************/
enum MENU_code fSetBeep( void )
{
    MENU_Question( "Beep On ?", &AUDIO_BuzzerOn );

    return MENU_CHANGE;
}

/*******************************************************************************
*
*                                fSetMenuFont_Ini
*
*******************************************************************************/
/**
*  Make the list of the fonts.
*
*  @return  MENU_CONTINUE_COMMAND
*
**/
/******************************************************************************/
enum MENU_code fSetMenuFont_Ini( void )
{
    int i,j = 0;

    FontList.LgMax = Font_Table.nb;
    for ( i = 0 ; i <= FontList.LgMax ; i++ )
    {
        if ((i < 3))                       /* Selection of font for the menu */
            FontList.Items[j++].Text = Font_Table.fonts[i].title;
    }

    LIST_Set( &FontList, 0, 0, 1 );
    return MENU_CONTINUE_COMMAND;
}

/*******************************************************************************
*
*                                fSetMenuFont_Mgr
*
*******************************************************************************/
/**
*  Set the menu font.
*
*  @return  MENU_CONTINUE_COMMAND
*
**/
/******************************************************************************/
enum MENU_code fSetMenuFont_Mgr( void )
{
    index_t FontSel=-1;

    // Manage the touchcreen list
    FontSel = LIST_Manager();

    if ( (FontSel != -1) )
    {
        Menu_Font = FontSel;
        return fQuit();
    }

    return MENU_CONTINUE;
}

/*******************************************************************************
*
*                                fInterface
*
*******************************************************************************/
/**
*  Launch the general menu for configuration.
*
*  @retval  MENU_CHANGE
*
**/
/******************************************************************************/
enum MENU_code fInterface( void )
{
    
    MENU_Set( &InterfaceMenu );

    return MENU_CHANGE;
}

/*******************************************************************************
*
*                                fPower
*
*******************************************************************************/
/**
*  Launch the general menu for Power management.
*
*  @retval  MENU_CHANGE
*
**/
/******************************************************************************/
enum MENU_code fPower( void )
{
    MENU_Set( &PowerMenu );

    return MENU_CHANGE;
}

/*******************************************************************************
*
*                                fSetAutorun
*
*******************************************************************************/
/**
*  Set if the application autorun or not
*
*  @return  MENU_CHANGE
*
**/
/******************************************************************************/
enum MENU_code fSetAutorun( void )
{
    MENU_Question( "Autorun ?", &AutorunOn );

    return MENU_CHANGE;
}


/*******************************************************************************
*
*                                SetCurrentApp
*
*******************************************************************************/
/**
*  Display in main menu the current application
*
*  @return
*
**/
/******************************************************************************/
void MENU_SetCurrentApp( void )
{
    tMenuItem*     curapp = 0;
    long unsigned  addr;
    int            CurrentApplication   = 0;

    ApplicationTable  = ( tMenuItem * ((* )[] ) )( *( unsigned long* )CIRCLEOS_FAT_ADDRESS );

    CurrentApplication = UTIL_ReadBackupRegister( BKP_SYS1 );

    if ( CurrentApplication > MAXAPP )
    {
        CurrentApplication = 0;
    }

    // First, we search for extra applications;
    if (( *ApplicationTable )[-CurrentApplication] != APP_VOID )
    {
        addr  = ( long unsigned )( *ApplicationTable )[-CurrentApplication];
        addr &= 0x00FFFFFF;
        addr |= 0x08000000;
        curapp = ( tMenuItem* ) addr;

        MainMenu.Items[1].Text = curapp->Text;
        MainMenu.Items[1].Fct_Init = curapp->Fct_Init;
        MainMenu.Items[1].Fct_Manage = curapp->Fct_Manage;
        MainMenu.Items[1].fMenuFlag = curapp->fMenuFlag;
    }
    else
    {
        MainMenu.Items[1].Text = "----------";
        MainMenu.Items[1].Fct_Init = MENU_Quit;
    }
}

/*******************************************************************************
*
*                                DB_LED_Init
*
*******************************************************************************/
/**
*
*  Initialization of the GPIOs for the LEDs of the daughter card
*
**/
/******************************************************************************/
void DB_LED_Init( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable LED GPIO clock */
    RCC_APB2PeriphClockCmd( RCC_APBxPeriph_GPIOx_DB_LED, ENABLE );

    /* Configure LED pins as output push-pull */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_DB_LED1 | GPIO_Pin_DB_LED2 ;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init( GPIOx_DB_LED, &GPIO_InitStructure );

    // Turn off LED's
    GPIO_WriteBit( GPIOx_DB_LED, GPIO_Pin_DB_LED1, Bit_SET );
    GPIO_WriteBit( GPIOx_DB_LED, GPIO_Pin_DB_LED2, Bit_SET );

}


/// @endcond