/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     menu_settings.c
* @brief    Management of a menu that allows to set up the STM32-primer.
* @author   FL
* @date     07/2007
* @version  4.0 Add Open4 Primer
* @date     10/2009
*
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
#if LCD_HIGH_DEF
#define PACK_SQUARE  (Char_Width * DRAW_GetCharMagniCoeff() * 3)
#else
#define PACK_SQUARE  (Char_Width * DRAW_GetCharMagniCoeff() * 2)
#endif

/* Private variables ---------------------------------------------------------*/
CONST_DATA u8                       max_values[]            = { 24, 60, 60 };
CONST_DATA u8* CONST_DATA           Test_Melody             = "Test:d=16,o=6,b=140:a,b,c,d,e,f,g";
CONST_DATA uint_t                   Speed_Range[5]          =
{ SPEED_VERY_LOW, SPEED_LOW, SPEED_MEDIUM, SPEED_HIGH, SPEED_VERY_HIGH };

u8                           temp_values[3];   // Holds the hour, minutes and seconds
index_t                      xCurSel;
index_t                      yCurSel;
bool                         FirstDisplay            = 1;
const u8*                    SetLevel_Title;
bool                         fDynamicChange          = 0;
divider_t                    menu_divider_coord      = 0;
Rotate_H12_V_Match_TypeDef   previous_Screen_Orientation;


/* Private function prototypes -----------------------------------------------*/

/* External ------------------------------------------------------------------*/
#if BACKLIGHT_INTERFACE
extern uint_t               Current_CCR_BackLightStart;
extern CONST_DATA int_t     CCR_BackLight_Tab[5];
#endif // BACKLIGHT_INTERFACE
extern tMenu                ConfigMenu;

/*******************************************************************************
*
*                                fSetPllRange_Mgr
*
*******************************************************************************/
/**
*  Set the level of the stable position calling MENU_SetLevel_Mgr()
*
*  @return  Value returned by MENU_SetLevel_Mgr()
*
**/
/******************************************************************************/
NODEBUG2 enum MENU_code fSetPllRange_Mgr( void )
{
    enum MENU_code ret;
    uint_t curspeed = CurrentSpeed; // Necessary because enums can have char type

    MENU_SetLevelTitle( " Freq Setting" );

    fDynamicChange = 0;
    ret            = MENU_SetLevel_Mgr( &curspeed, ( uint_t* )Speed_Range );
    CurrentSpeed = curspeed;
    UTIL_SetPll( CurrentSpeed );

    return ret;
}

#if BACKLIGHT_INTERFACE
/*******************************************************************************
*
*                                fSetBacklight_Mgr
*
*******************************************************************************/
/**
*  Set the level of the backlight calling MENU_SetLevel_Mgr()
*
*  @return  Value returned by MENU_SetLevel_Mgr()
*
**/
/******************************************************************************/
NODEBUG2 enum MENU_code fSetBacklight_Mgr( void )
{
    enum MENU_code ret;

    MENU_SetLevelTitle( "Backlight Power" );

    fDynamicChange = 1;
    ret            = MENU_SetLevel_Mgr( &Current_CCR_BackLightStart, ( uint_t* ) CCR_BackLight_Tab ) ;

    return ret;
}
#endif // BACKLIGHT_INTERFACE


/*******************************************************************************
*
*                                fSetTime_Ini
*
*******************************************************************************/
/**
*  Initialization function for time setting application.
*
*  @retval  MENU_CONTINUE_COMMAND
*
**/
/******************************************************************************/
NODEBUG2 enum MENU_code fSetTime_Ini( void )
{
    u8 THH;
    u8 TMM;
    u8 TSS;

    // No more screen rotation allowed.
    LCD_SetRotateScreen( 0 );

    // Get current time.
    RTC_GetTime( &THH, &TMM, &TSS );

    // Store values into global variables for later use in fSetTime_Mgr.
    temp_values[0] = THH;
    temp_values[1] = TMM;
    temp_values[2] = TSS;

    FirstDisplay = 1;

    POINTER_SetMode( POINTER_OFF );
    BUTTON_SetMode( BUTTON_ONOFF );
    MENU_Remove();

    xCurSel = -1;
    yCurSel = -1;

    return MENU_CONTINUE_COMMAND;
}

/*******************************************************************************
*
*                                fSetTime_Mgr
*
*******************************************************************************/
/**
*  Time setting application.
*
*  @retval  MENU_CONTINUE  Keep on calling me.
*  @retval  MENU_LEAVE     Application is over.
*
**/
/******************************************************************************/
NODEBUG2 enum MENU_code fSetTime_Mgr( void )
{
    bool fRedrawn = 0;
    u8 buffer[10];
    u8 i;
    coord_t Time_Title_H;
    coord_t CharHeight, CharWidth;
    coord_t pos;
    coord_t ysel;
    coord_t xsel;

    // Hide menu.
    CurrentMenu = 0;

    // Character magnify deponding on the platform !
    DRAW_RestoreCharMagniCoeff();

    CharHeight = Char_Height * DRAW_GetCharMagniCoeff();
    CharWidth = Char_Width * DRAW_GetCharMagniCoeff();

Restart:
    if ( FirstDisplay )
    {
        previous_Screen_Orientation = LCD_GetScreenOrientation();

        if ( fRedrawn == 0 )
        {
            POINTER_SetMode( POINTER_OFF );
        }

        Time_Title_H = CharHeight * 2;
        LCD_FillRect_Circle( 0, Screen_Height - Time_Title_H, Screen_Width, Time_Title_H, RGB_BLACK );

        DRAW_SetBGndColor( RGB_BLACK );
        DRAW_SetTextColor( RGB_MAKE( 0, 183, 239 ) );
        DRAW_DisplayStringWithMode( 0, Screen_Height - (( 3 * CharHeight ) / 2 ), "Time setting", ALL_SCREEN, NORMAL_TEXT, CENTER );
        DRAW_SetDefaultColor();

        // Display current time.
        for ( i = 0 ; i < 3 ; i ++ )
        {
            UTIL_uint2str( buffer, temp_values[i], 2, 1 );

            DRAW_DisplayString( TIME_MARGIN + i *( PACK_SQUARE + 2 ), TIME_BOTTOM, buffer, 2 );
            DRAW_DisplayString( TIME_MARGIN + i *( PACK_SQUARE + 2 ), TIME_BOTTOM + ( CharHeight + 2 ), "++", 2 );
            DRAW_DisplayString( TIME_MARGIN + i *( PACK_SQUARE + 2 ), TIME_BOTTOM - ( CharHeight + 2 ), "--", 2 );
        }

        // Draw a big X for the 'cancel' button.
        DRAW_SetTextColor( RGB_RED );
        DRAW_SetCharMagniCoeff( 2 );
        DRAW_DisplayString( TIME_MARGIN + ( -1 ) *( PACK_SQUARE + 2 ) , LCD_HIGH_DEF ? TIME_BOTTOM : TIME_BOTTOM - 6, "X", 1 );

        // Draw a big V for the 'validate' button.
        DRAW_SetTextColor( RGB_BLUE );
#if LCD_HIGH_DEF
        DRAW_DisplayString( TIME_MARGIN + 3 *( PACK_SQUARE + 2 ) + 10, TIME_BOTTOM, "V", 1 );
#else
        DRAW_DisplayString( TIME_MARGIN + 3 *( PACK_SQUARE + 2 ) - 30, TIME_BOTTOM - 6, "V", 1 );
#endif

        // Restore magnify coefficient.
        DRAW_RestoreCharMagniCoeff();
        DRAW_SetDefaultColor();

        // Restrict the pointer moves to a rectangle.
        POINTER_SetRect( TIME_MARGIN + ( -1 ) *( PACK_SQUARE + 2 ), TIME_BOTTOM - ( CharHeight + 2 ),
        5 *( PACK_SQUARE + 2 ) - POINTER_WIDTH,
        3 *( CharHeight + 2 ) - POINTER_WIDTH );

        POINTER_SetMode( POINTER_MENU );
        fRedrawn = 1;
    }

    pos = POINTER_GetPos();
    ysel = ((( pos >> 8 ) & 0xFF ) - ( TIME_BOTTOM - ( CharHeight + 2 ) ) ) / ( CharHeight + 2 );
    xsel;
    xsel = (( pos & 0xFF ) - ( TIME_MARGIN + 10 - ( CharWidth + 2 ) ) ) / ( CharWidth + 2 );

    // ysel value meanings:
    // 0 : decrement line.
    // 1 : X and V line.
    // 2 : increment line.

    // xsel value meanings:
    // -1: on the 'cancel' button.
    //  0: hours
    //  1: minutes
    //  2: seconds
    //  3: on the 'validate' button.

    // xsel can only be in the [-1 .. 3] range.
    if ( xsel > 3 )
    {
        xsel = 3;
    }

    if ( xsel < -1 )
    {
        xsel = -1;
    }

    if (( xsel == -1 ) || ( xsel == 3 ) )
        ysel = 1;


    // The user pushed the button. Action will be different according to the
    // cursor position (xsel).
    if ( BUTTON_GetState() == BUTTON_PUSHED )
    {
        BUTTON_WaitForRelease();

        switch ( xsel )
        {
            // Set time with modified values before leaving application.
        case 3   :
            RTC_SetTime( temp_values[0], temp_values[1], temp_values[2] );

            // Leave application restoring CircleOS state.
        case -1  :
            MENU_ClearCurrentCommand();
            DRAW_SetDefaultColor();
            DRAW_Clear();
            POINTER_SetMode( POINTER_ON );
            BUTTON_SetMode( BUTTON_ONOFF_FORMAIN );
            POINTER_SetRectScreen();
            DRAW_SetCharMagniCoeff( 1 );
            LCD_SetRotateScreen( 1 );
            return MENU_LEAVE;

            // Increment or decrement according to ysel.
        case 0   :
        case 1   :
        case 2   :
            // Decrement
            if ( ysel == 0 )
            {
                // If selected value is null roll over to max value.
                if ( temp_values[xsel] == 0 )
                {
                    temp_values[xsel] = max_values[xsel] - 1;
                }
                else
                {
                    temp_values[xsel]--;
                }
            }
            else if ( ysel == 2 ) // increment
            {
                temp_values[xsel]++;

                // Maximum value reached, restart at 0.
                if ( temp_values[xsel] == max_values[xsel] )
                {
                    temp_values[xsel] = 0;
                }
            }
            break;
        }

        // Something was changed, redraw screen!
        FirstDisplay = 1;
        fRedrawn     = 1;

        goto Restart;
    }

    // Cursor changed its position or a redraw is needed.
    if (( xsel != xCurSel ) || ( ysel != yCurSel ) || fRedrawn )
    {
        if ( FirstDisplay == 0 )
        {
            // Unselect previously selected button.
            if ( xCurSel == -1 )
            {
                // 'Cancel' button.
                DRAW_SetCharMagniCoeff( 2 );
                DRAW_SetTextColor( RGB_RED );
                DRAW_DisplayString( TIME_MARGIN + ( -1 ) *( PACK_SQUARE + 2 ), LCD_HIGH_DEF ? TIME_BOTTOM : TIME_BOTTOM - 6, "X", 1 );

                // Restore default settings.
                DRAW_SetDefaultColor();
                DRAW_RestoreCharMagniCoeff();
            }
            else if ( xCurSel == 3 )
            {
                // 'Validate' button.
                DRAW_SetCharMagniCoeff( 2 );
                DRAW_SetTextColor( RGB_BLUE );
#if LCD_HIGH_DEF
                DRAW_DisplayString( TIME_MARGIN + 3 *( PACK_SQUARE + 2 ) + 10, TIME_BOTTOM, "V", 1 );
#else
                DRAW_DisplayString( TIME_MARGIN + 3 *( PACK_SQUARE + 2 ) - 30, TIME_BOTTOM - 6, "V", 1 );
#endif

                // Restore default settings.
                DRAW_SetDefaultColor();
                DRAW_RestoreCharMagniCoeff();
            }
            else
            {
                switch ( yCurSel )
                {
                case 1 :
                    UTIL_uint2str( buffer, temp_values[xCurSel], 2, 1 );
                    DRAW_DisplayString( TIME_MARGIN + xCurSel *( PACK_SQUARE + 2 ), TIME_BOTTOM, buffer, 2 );
                    break;

                case 0 :
                    DRAW_DisplayString( TIME_MARGIN + xCurSel *( PACK_SQUARE + 2 ), TIME_BOTTOM - ( CharHeight + 2 ), "--", 2 );
                    break;

                case 2 :
                    DRAW_DisplayString( TIME_MARGIN + xCurSel *( PACK_SQUARE + 2 ), TIME_BOTTOM + ( CharHeight + 2 ), "++", 2 );
                    break;
                }
            }
        }

        // Select new selection.
        if ( xsel == -1 )
        {
            // 'Cancel' button.
            DRAW_SetCharMagniCoeff( 2 );
            DRAW_DisplayStringInverted( TIME_MARGIN + ( -1 ) *( PACK_SQUARE + 2 ), LCD_HIGH_DEF ? TIME_BOTTOM : TIME_BOTTOM - 6, "X", 1 );
            DRAW_RestoreCharMagniCoeff();
        }
        else if ( xsel == 3 )
        {
            // 'Validate' button.
            DRAW_SetCharMagniCoeff( 2 );
#if LCD_HIGH_DEF
            DRAW_DisplayStringInverted( TIME_MARGIN + 3 *( PACK_SQUARE + 2 ) + 10, TIME_BOTTOM, "V", 1 );
#else
            DRAW_DisplayStringInverted( TIME_MARGIN + 3 *( PACK_SQUARE + 2 ) - 30, TIME_BOTTOM - 6, "V", 1 );
#endif
            DRAW_RestoreCharMagniCoeff();
        }
        else
        {
            switch ( ysel )
            {
                // Time
            case 1 :
                UTIL_uint2str( buffer, temp_values[xsel], 2, 1 );
                DRAW_DisplayStringInverted( TIME_MARGIN + xsel *( PACK_SQUARE + 2 ), TIME_BOTTOM, buffer, 2 );
                break;

                // --
            case 0 :
                DRAW_DisplayStringInverted( TIME_MARGIN + xsel *( PACK_SQUARE + 2 ), TIME_BOTTOM - ( CharHeight + 2 ), "--", 2 );
                break;

                // ++
            case 2 :
                DRAW_DisplayStringInverted( TIME_MARGIN + xsel *( PACK_SQUARE + 2 ), TIME_BOTTOM + ( CharHeight + 2 ), "++", 2 );
                break;
            }
        }

        xCurSel = xsel;
        yCurSel = ysel;
    }

    FirstDisplay = 0;

    return MENU_CONTINUE;

}


/* Public functions for CircleOS ---------------------------------------------*/

/*******************************************************************************
*
*                                fConfig
*
*******************************************************************************/
/**
*  Launch the general menu for configuration.
*
*  @retval  MENU_CHANGE
*
**/
/******************************************************************************/
NODEBUG2 enum MENU_code fConfig( void )
{
    MENU_Set(( tMenu* )&ConfigMenu );

    return MENU_CHANGE;
}


/// @endcond

/* Public functions ----------------------------------------------------------*/

/*******************************************************************************
*
*                                MENU_SetLevel_Ini
*
*******************************************************************************/
/**
*
*  Initialize a generic function to set a value in the range of [0,4]
*
*  @retval  MENU_CONTINUE_COMMAND
*
**/
/******************************************************************************/
enum MENU_code MENU_SetLevel_Ini( void )
{
    FirstDisplay = 1;

    MENU_Remove();
    BUTTON_SetMode( BUTTON_ONOFF );
    POINTER_SetMode( POINTER_OFF );
    DRAW_Clear();

    return MENU_CONTINUE_COMMAND;
}

/*******************************************************************************
*
*                                MENU_SetLevelTitle
*
*******************************************************************************/
/**
*
*  Set the title of level menu managed by MENU_SetLevel_Mgr()
*
*  @param[in] title : A pointer to the string containing the level menu title.
*
**/
/******************************************************************************/
void MENU_SetLevelTitle( const u8* title )
{
    SetLevel_Title = title;
}

/*******************************************************************************
*
*                                MENU_SetLevel_Mgr
*
*******************************************************************************/
/**
*
*  Generic function to set a value in the range of [0,4] (handling of the control)
*
*  @param[in,out] value        :  Pointer to the value to be modified.
*  @param[in]     value_range  :  Pointer to the list of 5 valid values.
*
*  @return  Menu code
*
*  If the variable fDynamicChange is set, the value is changed as the
*  selection change.
*
**/
/********************************************************************************/
enum MENU_code MENU_SetLevel_Mgr( uint_t* value, uint_t* value_range )
{
    u8 sel;
    bool fRedrawn = 0;

    // Hide menu
    CurrentMenu = 0;

    // No character magnify.
    DRAW_RestoreCharMagniCoeff();

    // Disable Screen orientation
    LCD_SetRotateScreen( 0 );

    if ( FirstDisplay )
    {
        u8 i;
        u8 buffer[2] = {0};

        CurSel = 0;
        for ( i = 0 ; i < 5; i++ )
        {
            if ( *value == value_range[i] )
            {
                CurSel = i;
                break;
            }
        }

        POINTER_SetMode( POINTER_OFF );
        DRAW_DisplayStringWithMode( 0, BAR_BOTTOM + ( 3 * WIDTH_SQUARE ), SetLevel_Title, ALL_SCREEN, NORMAL_TEXT, CENTER );
        POINTER_SetPos( BAR_LEFT + ( WIDTH_SQUARE + 2 ) * CurSel + WIDTH_SQUARE / 2, BAR_BOTTOM );
        FirstDisplay = 0;

        for ( i = 0; i < MAXLEVEL; i++ )
        {
            *buffer = '0' + i;

            LCD_FillRect_Circle( BAR_LEFT + i *( WIDTH_SQUARE + 2 ), BAR_BOTTOM, WIDTH_SQUARE, WIDTH_SQUARE, 0x0f0f );

            // Display value.
            DRAW_DisplayString( BAR_LEFT + i *( WIDTH_SQUARE + 2 ) + WIDTH_SQUARE / 2 - ( Char_Width * DRAW_GetCharMagniCoeff() ) / 2,
            BAR_BOTTOM + WIDTH_SQUARE + WIDTH_SQUARE / 2,
            buffer,
            1 );
        }

        // Restrict the pointer moves to a rectangle.
        POINTER_SetRect( BAR_LEFT, BAR_BOTTOM, MAXLEVEL *( WIDTH_SQUARE + 2 ) - POINTER_WIDTH, WIDTH_SQUARE - POINTER_WIDTH );
        POINTER_SetMode( POINTER_MENU );

        fRedrawn = 1;
    }

    // Change current selection
    sel = (( POINTER_GetPos() & 0xFF ) - BAR_LEFT ) / ( WIDTH_SQUARE + 2 );

    // The user pushed the button!
    if ( BUTTON_GetState() == BUTTON_PUSHED )
    {
        BUTTON_WaitForRelease();

        // The returned value is the selected one!
        *value = value_range[CurSel];

        // Reset CircleOS state.
        MENU_ClearCurrentCommand();
        DRAW_SetDefaultColor();
        DRAW_Clear();
        POINTER_SetMode( POINTER_ON );
        BUTTON_SetMode( BUTTON_ONOFF_FORMAIN );
        POINTER_SetRectScreen();
        DRAW_SetCharMagniCoeff( 1 );
        LCD_SetRotateScreen( 1 );

        return MENU_LEAVE;
    }

    if ((( menu_divider_coord++ % 5 ) != 0 ) && !fRedrawn )
    {
        return MENU_CONTINUE;
    }

    // Redraw
    if (( sel != CurSel ) || fRedrawn )
    {
        // Clear previously selected value.
        if ( CurSel >= 0 )
        {
            LCD_FillRect_Circle( BAR_LEFT + CurSel *( WIDTH_SQUARE + 2 ), BAR_BOTTOM, WIDTH_SQUARE, WIDTH_SQUARE, 0x0f0f );
        }

        // Draw newly selected value.
        LCD_FillRect_Circle( BAR_LEFT + sel *( WIDTH_SQUARE + 2 ), BAR_BOTTOM, WIDTH_SQUARE, WIDTH_SQUARE, 0x0f0f + 2 );

        CurSel = sel;

        // If dynamic change,
        if ( fDynamicChange )
        {
            *value = value_range[CurSel];
        }
    }

    return MENU_CONTINUE;
}
