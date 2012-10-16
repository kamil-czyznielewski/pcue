/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     menu.c
* @brief    General management for menu, dialog boxes ...
* @author   FL
* @date     07/2007
*
* @date     10/2008
* @version  3.0 Add Primer 2 and ST library v2.0.3

* @version  4.0 Add Open4 Primer
* @date     10/2009 
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Public function prototypes ------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define MENU_DIVIDER    100
#define MENU_DIVIDER2   10          // Divider for list management PRIMER2
#define DELAY_AFTER_SEL 80          /*!< Disabling of MEMS after selection with joystick/touchscreen  */
#define TIME_FOR_TOUCH_MENU  (2000)
#define MENU_MIDD_POS   (-350)

/* Private function prototypes -----------------------------------------------*/
void MENU_SetCurrentApp( void );
enum MENU_code fYes( void );
enum MENU_code fNo( void );

/* External variables --------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
//u16                                 CharMagniCoeff_Menu           = 1;
color_t                             BGndColor_Menu                = RGB_MAKE( 0xe6, 0xe6, 0xe6 ); //RGB_GREEN;
color_t                             TextColor_Menu                = RGB_MAKE( 0x0, 0x0, 0x0 ); //RGB_BLUE;
tMenu*                              CurrentMenu                   = 0;
tMenuItem*                          CurrentCommand                = 0;
tMenuItem*                          OldCommand                    = 0;
divider_t                           divider                       = 0;
bool                                iToShutDown                   = 0;
index_t                             CurSel                        = -1;    /*!< Current selected.                   */
index_t                             Oldsel                        = 0;     /*!< Previous immediate position.        */
index_t                             Newsel                        = 0;     /*!< Match with the immediate position.  */
u8                                  MenuCharWidth                 = 7;
u8                                  MenuCharHeight                = 14;
s32 MenuTimePressed = 0;
s16                                 DoubleClickCounter_Menu       = 0;     /*!< Current DblClick counter.           */
bool                                JoystickAsInput               = 1;
bool                                MemsAsInput                   = 1;
bool                                TchscrAsInput                 = 1;
static s32                          Counter_Joystick_Touch        = 0;
divider_t Menu_Divider          = MENU_DIVIDER;
divider_t Appli_Divider         = MENU_DIVIDER;
color_t title_BGndColor         = RGB_MAKE( 0xE6, 0x0, 0x0 );
color_t title_TextColor         = RGB_WHITE;
color_t selected_BGndColor      = RGB_MAKE( 0x8C, 0xBE, 0x0 );
color_t selected_TextColor      = RGB_WHITE;
tMenu* oldCurrentMenu;
extern tMenu AppMenu;

tMenu QuestionMenu =
{
    1,
    0,
    2, 0, 0, 0, 0, 0,
    0,
    {
        { "Yes",    fYes,       0,  0 },
        { "No",     fNo,        0,  0 }
    }
};

tMenu PrintMenu =
{
    1,
    0,
    0, 0, 0, 0, 0, 0,
    0,
    {
        { 0, 0, 0, 0 },
        { 0, 0, 0, 0 }
    }
};

#if TOUCHSCREEN_AVAIL
tListMin ListMenu =
{
    1,
    "Title",
    MENU_MAXITEM,
    0, 0, 0, 0, 0,
    MENU_MAXITEM,
    0,
    0
};
#endif

bool* CurrentAnswer = 0;

/* Private functions ---------------------------------------------------------*/
enum MENU_code fQuit( void );

/*******************************************************************************
*
*                                fShutDown
*
*******************************************************************************/
/**
*  Switch off the power supply.
*
*  @retval  MENU_CHANGE (but dead before returning).
*
**/
/******************************************************************************/
NODEBUG2 enum MENU_code fShutdown( void )
{
    MENU_Question( "Shutdown?", &iToShutDown );

    return MENU_CHANGE;
}

/*******************************************************************************
*
*                                fSDCard
*
*******************************************************************************/
/**
*  Display SDCard menu.
*
*  @retval  MENU_CHANGE.
*
**/
/******************************************************************************/
#if SDCARD_AVAIL
NODEBUG2 enum MENU_code fSDCard( void )
{
    MENU_Set( &SDcardMenu );

    return MENU_CHANGE;
}
#endif


/*******************************************************************************
*
*                                RefreshItem
*
*******************************************************************************/
/**
*  Display the text of the command.
*
*  @param[in]  sel         The index of the item in the current menu.
*  @param[in]  isInverted  When non null, text is displayed inverted.
*
**/
/******************************************************************************/
#if !TOUCHSCREEN_AVAIL
NODEBUG2 void RefreshItem( index_t sel, bool isInverted )
{
    color_t save_BGndColor      = DRAW_GetBGndColor();
    color_t save_TextColor      = DRAW_GetTextColor();

    // Set menu DRAW params
    LCD_ChangeFont(Menu_Font);
    DRAW_SetBGndColor(( sel == -1 ) ? title_BGndColor : BGndColor_Menu );
    DRAW_SetTextColor(( sel == -1 ) ? title_TextColor : TextColor_Menu );

    MenuCharWidth = Char_Width * CharMagniCoeff;
    MenuCharHeight = Char_Height * CharMagniCoeff;

    // Standard display
    if ( !isInverted )
    {
        DRAW_DisplayString(( Char_Width / 2 ) + CurrentMenu->XPos + 0,
                           ( Char_Width / 2 ) + CurrentMenu->YPos + ( CurrentMenu->NbItems - sel - 1 ) * MenuCharHeight,
                           ( sel == -1 ) ? ( u8* )CurrentMenu->Title : ( u8* )( CurrentMenu->Items[sel].Text ),
                           CurrentMenu->LgMax );
    }
    else
    {
        DRAW_SetBGndColor(( sel == CurrentMenu->SelectedItem ) ? selected_BGndColor : title_BGndColor );
        DRAW_SetTextColor(( sel == CurrentMenu->SelectedItem ) ? selected_TextColor : title_TextColor );

        // Inverted display.
        DRAW_DisplayString(( Char_Width / 2 ) + CurrentMenu->XPos,
                           ( Char_Width / 2 ) + CurrentMenu->YPos + ( CurrentMenu->NbItems - sel - 1 ) * MenuCharHeight,
                           ( sel == -1 ) ? ( u8* ) CurrentMenu->Title : ( u8* )( CurrentMenu->Items[sel].Text ),
                           CurrentMenu->LgMax );
    }

    // Draw a white rectangle around selected item, or background color for the others itmes.
    LCD_DrawRect( CurrentMenu->XPos + 2,
                  ( Char_Width / 2 ) + CurrentMenu->YPos + ( CurrentMenu->NbItems - sel - 1 ) * MenuCharHeight,
                  CurrentMenu->LgMax  * MenuCharWidth + 1,
                  Char_Height * CharMagniCoeff,
                  ( isInverted ) ? RGB_WHITE : BGndColor_Menu );

    // Restore previous DRAW params.
    LCD_SetDefaultFont();
    DRAW_SetBGndColor( save_BGndColor );
    DRAW_SetTextColor( save_TextColor );
}
#endif


/*******************************************************************************
*
*                                fYes
*
*******************************************************************************/
/**
*  Records the answer 'Yes' for the current question.
*
*  @retval  MENU_LEAVE.
*
**/
/******************************************************************************/
NODEBUG2 enum MENU_code fYes( void )
{
    if ( CurrentAnswer )
    {
        *CurrentAnswer = 1;
    }

    return fQuit();
}

/*******************************************************************************
*
*                                fNo
*
*******************************************************************************/
/**
*  Records the answer 'No' for the current question.
*
*  @retval  MENU_LEAVE from fQuit()
*
**/
/******************************************************************************/
NODEBUG2 enum MENU_code fNo( void )
{
    if ( CurrentAnswer )
    {
        *CurrentAnswer = 0;
    }
    return fQuit();
}


/* Public functions for CircleOS ---------------------------------------------*/

/*******************************************************************************
*
*                                MENU_Handler
*
*******************************************************************************/
/**
*
*  Called by the CircleOS scheduler to manage the current menu.
*  It is the MENU_Handler that calls application initialization function and
*  application 'main' functions.
*
**/
/******************************************************************************/
void MENU_Handler( void )
{
    int            ret;
    static u32     bdelay = 0;
    index_t        app;

    if ( fInitDone  == FALSE )
        return;

    //-----------------------------------------------------------------------------
    // In the 'home' page, the push button calls the main menu
    //-----------------------------------------------------------------------------
    if (( CurrentMenu == 0 ) && ( BUTTON_GetState() == BUTTON_PUSHED_FORMAIN ) )
    {

        BUTTON_WaitForRelease();

        // Add current application into main menu
        MENU_SetCurrentApp();

        // Display main menu
        MENU_Set( &MainMenu );
    }

    //-----------------------------------------------------------------------------
    // iToShutDown is a global flag to indicate that a shutdown has been requested
    // In any situation, a long push will shutdown the power.
    // This part is managed by the power handler if exists
    //-----------------------------------------------------------------------------
    #if !POWER_MNGT
    if ( iToShutDown )
    {   
        while(1)
        {
            SHUTDOWN_Action();
        }
    }
    #endif

    //-----------------------------------------------------------------------------
    // No active menu (nor command) => nothing to do in this handler
    //-----------------------------------------------------------------------------
    if (( CurrentMenu == 0 ) && ( CurrentCommand == 0 ) )
    {
        return;
    }


    //-----------------------------------------------------------------------------
    // When a menu is active, we check whether a command is to be launched
    //-----------------------------------------------------------------------------
#if !TOUCHSCREEN_AVAIL
    if (( CurrentMenu != 0 )    &&
            ( CurrentCommand == 0 ) &&
            (( BUTTON_GetState() == BUTTON_PUSHED_FORMAIN ) ||
             ( DoubleClickCounter_Menu != MEMS_Info.DoubleClick )
            ) )
    {
#endif

#if TOUCHSCREEN_AVAIL

        //----------------------------------------------------------------------------------
        // Continue only each MENU_DIVIDER2 calls (specific for the LIST management).
        //----------------------------------------------------------------------------------
        divider++;
        if ( divider % MENU_DIVIDER2 )
        {
            return;
        }

        if (( CurrentMenu != 0 ) && ( CurrentCommand == 0 ) )
        {
            index_t index = -1;

            // Management menu by List
            index = LIST_Manager();

            // Item selected
            if ( index != -1 )
            {
                CurrentMenu->SelectedItem = index;
#endif
                oldCurrentMenu = CurrentMenu;

                BUTTON_WaitForRelease();
                DoubleClickCounter_Menu = MEMS_Info.DoubleClick;

                // Handle the application change selection
                if ( ( CurrentMenu == &AppMenu )
                  && ( AppMenu.Items[AppMenu.SelectedItem].Fct_Init != MENU_Quit ) )
                {
                    app = APP_FindAppIndex( CurrentMenu->SelectedItem );
                    UTIL_WriteBackupRegister( BKP_SYS1, app );
                    UTIL_LoadApp( app );
                }

                CurrentCommand = &( oldCurrentMenu->Items[oldCurrentMenu->SelectedItem] );

                if ( CurrentCommand->fMenuFlag & REMOVE_MENU )
                {
                    MENU_Remove();
                }

                if ( oldCurrentMenu->NbItems )
                {
                    if ( CurrentCommand->fMenuFlag & APP_MENU )
                    {
                        MENU_Remove();
                        POINTER_SetMode( POINTER_OFF );
                        BUTTON_SetMode( BUTTON_ONOFF );
                        LCD_SetRotateScreen( 0 );

                        // By default, set the offset for application compatibility with Open4
                        if ( UTIL_GetPrimerType() > 2 )
                            LCD_SetOffset( OFFSET_ON );
                    }

                    // Call application initialization function.
#if POWER_MNGT
                    POWER_Reset_Time();
#endif
                    ret = CurrentCommand ->Fct_Init();

                    MENU_ClearCurrentCommand();

                    if ( ret != MENU_CHANGE )
                    {
                        if ( ret != MENU_CONTINUE_COMMAND )
                        {
                            CurrentMenu = oldCurrentMenu;
                        }
                    }
                }
                else
                {
                    CurrentMenu = 0;

                    MENU_ClearCurrentCommand();
                    MENU_Remove();
                    fQuit();

                    return;
                }

                switch ( ret )
                {
                case MENU_LEAVE_AS_IT      :
                    CurrentMenu = 0;
                    CurrentCommand = 0;
                    LCD_SetOffset( OFFSET_OFF );
                    LCD_SetDefaultFont();
                    LCD_SetTransparency(0);
                    break;

                case MENU_LEAVE            :
                    if ( !( CurrentMenu->Items[CurrentMenu->SelectedItem].fMenuFlag & REMOVE_MENU ) )
                    {
                        MENU_Remove();
                    }
                    CurrentCommand = 0;
                    CurrentMenu = 0;
                    LCD_SetOffset( OFFSET_OFF );
                    LCD_SetDefaultFont();
                    LCD_SetTransparency(0);
                    break;

                case MENU_REFRESH          :
                    MENU_Set( CurrentMenu );
                    break;

                case MENU_CHANGE           :
                case MENU_CONTINUE         :
                    break;

                case MENU_CONTINUE_COMMAND :
                    CurrentCommand = &( oldCurrentMenu->Items[oldCurrentMenu->SelectedItem] );
                    break;
                }

                return;

#if TOUCHSCREEN_AVAIL
            } // end if((index != -1))
#endif
        } // end if(CurrentMenu != 0)

#if !TOUCHSCREEN_AVAIL
        //-----------------------------------------------------------------------------
        // Continue only each Appli_Divider calls.
        //-----------------------------------------------------------------------------
        divider++;
#endif

        //-----------------------------------------------------------------------------
        // A command is active and we redirect the execution to the application (or the configuration procedure)
        //-----------------------------------------------------------------------------
        if ( CurrentCommand && CurrentCommand->Fct_Manage )
        {
            if ( divider % Appli_Divider )
            {
                return;
            }

            // Call application 'main' function.
#if POWER_MNGT
            POWER_Reset_Time();
#endif

            ret = CurrentCommand->Fct_Manage();

            if ( ret == MENU_LEAVE )
            {
                LCD_SetOffset( OFFSET_OFF );
                LCD_SetDefaultFont();
                LCD_SetTransparency(0);
                MENU_Remove();
                CurrentMenu = 0;
                CurrentCommand = 0;
                LCD_SetRotateScreen( 1 );           // YRT20090305
            }
            else if ( (ret == MENU_RESTORE_COMMAND) && OldCommand )
            {
                // YRT20090402 restore previous command,
                CurrentCommand  = OldCommand;       // like application for example
            }

            return;
        }

        //-----------------------------------------------------------------------------
        // Continue only each MENU_DIVIDER calls.
        //-----------------------------------------------------------------------------
        if ( divider % Menu_Divider )
        {
            return;
        }

        //-----------------------------------------------------------------------------
        // Check again whether a menu is active
        //-----------------------------------------------------------------------------
        if ( CurrentMenu == 0 )
        {
            return;
        }

#if !TOUCHSCREEN_AVAIL
        //-----------------------------------------------------------------------------
        // Manage the selection change within a menu
        //-----------------------------------------------------------------------------
#if JOYSTICK_AVAIL
        //-----------------------------------------------------------------------------
        // Manage the command selection with joystick according to interface configuration
        //-----------------------------------------------------------------------------
        if ( JoystickAsInput )
        {
#if POWER_MNGT
            POWER_Reset_Time();
#endif
            switch ( JOYSTICK_GetState() )
            {
            case JOYSTICK_DOWN:
                Newsel = Oldsel + 1;
                JOYSTICK_WaitForRelease();
                Counter_Joystick_Touch = WEIGHTED_TIME( DELAY_AFTER_SEL ); // to leave the hand to the joystick for a while
                break;

            case JOYSTICK_UP:
                Newsel = Oldsel - 1;
                JOYSTICK_WaitForRelease();
                Counter_Joystick_Touch = WEIGHTED_TIME( DELAY_AFTER_SEL ); // to leave the hand to the joystick for a while
                break;
            default:
                if ( Counter_Joystick_Touch )
                    Counter_Joystick_Touch-- ;
                break;
            }
        }

#endif // if Joystick  

        if (( MemsAsInput && ( Counter_Joystick_Touch == 0 ) ) || !JoystickAsInput )
        {
            // Manage the command selection with the current menu with mems
            s16 MenuMaxPosY = MENU_MIDD_POS - 20 * CurrentMenu->NbItems;
            s16 MenuMinPosY = MENU_MIDD_POS + 20 * CurrentMenu->NbItems;

            // Filtering of the moves in the menu:
            //  Oldsel and Newsel defines the new selected items.
            //  Newsel is the current selected item, and Oldsel the previous one.
            // When the selection is stable (Newsel==Oldsel for MAXBTIME times),
            //  we consider that the current selection (sel) can be changed by Newsel.

            // Try absolute angle for positionning.
            if ( MEMS_Info.RELATIVE_Y > MenuMinPosY )
            {
                // Out of range: too low.
                Newsel = 0;
            }
            else if ( MEMS_Info.RELATIVE_Y < MenuMaxPosY )
            {
                // Out of range: too high.
                Newsel = CurrentMenu->NbItems - 1;
            }
            else  // In the range
            {
                Newsel = (( MenuMinPosY  - MEMS_Info.RELATIVE_Y ) * ( CurrentMenu->NbItems - 2 ) ) / ( MenuMinPosY  - MenuMaxPosY ) + 1 ;
            }
        }

        //-----------------------------------------------------------------------------
        // Manage the command selection also with mems according to interface configuration
        //-----------------------------------------------------------------------------
        if ( Newsel >= CurrentMenu->NbItems )
        {
            Newsel = CurrentMenu->NbItems - 1;
        }

        if ( Newsel < 0 )
        {
            Newsel = 0;
        }

        if ( Newsel != Oldsel )
        {
            Oldsel = Newsel;
        }
        else
        {
            // Hysteresis
            bdelay++;

            if ( bdelay >= WEIGHTED_TIME( MAXBTIME ) )
            {
                CurSel = Newsel;
                bdelay = 0;
            }
        }

        if (( CurrentMenu->SelectedItem != CurSel ) && CurrentMenu->NbItems )
        {
            // First refresh old selected item
            if ( CurrentMenu->SelectedItem >= 0 )
            {
                RefreshItem( CurrentMenu->SelectedItem, NORMAL_TEXT );
            }

            // Then select the new selected item
            CurrentMenu->SelectedItem = CurSel;
            RefreshItem( CurrentMenu->SelectedItem, INVERTED_TEXT );
        }

#endif // if !Touchscreen    
    }

/*******************************************************************************
*
*                                fQuit
*
*******************************************************************************/
/**
*  Leaves current menu (stands for "cancel").
*
*  @retval  MENU_LEAVE.
*
**/
/******************************************************************************/
NODEBUG2 enum MENU_code fQuit( void )
    {
        BUTTON_WaitForRelease();
        DRAW_SetDefaultColor();

        LCD_SetOffset( OFFSET_OFF );

        POINTER_SetApplication_Pointer_Mgr( 0 );
        POINTER_SetMode( POINTER_OFF );
        POINTER_Init();

        MENU_ClearCurrentCommand();
        DRAW_Clear();
        LCD_SetDefaultFont();
        LCD_SetTransparency(0);

        LED_Set( LED_GREEN, LED_OFF );
        LED_Set( LED_RED, LED_OFF );

#ifdef SDCARD
        FS_SetPathFilter( 0 );
#endif

        POINTER_SetMode( POINTER_ON );
        LCD_SetRotateScreen( 1 );
        BUTTON_SetMode( BUTTON_ONOFF_FORMAIN );

        return MENU_LEAVE;
    }

/*******************************************************************************
*
*                                FS_Explorer_Handler
*
*******************************************************************************/
/**
*  Navigation into the SDCARD folders.
*
*  @retval  MENU_CONTINUE : if navigation.
*           MENU_LEAVE : if Quit requested.
*
**/
/******************************************************************************/
#if SDCARD_AVAIL
NODEBUG2 enum MENU_code FS_Explorer_Handler( void )
    {
        if ( FS_Explorer() == -1 )
            return MENU_CONTINUE;
        else
            return fQuit();
    }
#endif //SDCARD_AVAIL

/// @endcond

/* Public functions ----------------------------------------------------------*/

/*******************************************************************************
*
*                                MENU_Quit
*
*******************************************************************************/
/**
*
*  Leave the current menu (stand for "cancel") and clear screen.
*
*  @retval  MENU_LEAVE
**/
/******************************************************************************/
enum MENU_code MENU_Quit( void )
    {
        return fQuit();
    }

/*******************************************************************************
*
*                                MENU_SetTextColor
*
*******************************************************************************/
/**
*
*  Set the color used for text menu.
*
*  @param[in]  TxtColor New color used for menu text.
*
**/
/********************************************************************************/
void MENU_SetTextColor( color_t TxtColor )
    {
        TextColor_Menu = TxtColor;
    }

/*******************************************************************************
*
*                                MENU_GetTextColor
*
*******************************************************************************/
/**
*
*  Return the color used for text menu.
*
*  @return  Menu text color.
*
**/
/******************************************************************************/
color_t MENU_GetTextColor( void )
    {
        return TextColor_Menu;
    }

/*******************************************************************************
*
*                                MENU_SetBGndColor
*
*******************************************************************************/
/**
*
*  Set the background color used for menu.
*
*  @param[in]  BGndColor New menu background color.
*
**/
/******************************************************************************/
void MENU_SetBGndColor( color_t BGndColor )
    {
        BGndColor_Menu = BGndColor;
    }

/*******************************************************************************
*
*                                MENU_GetBGndColor
*
*******************************************************************************/
/**
*
*  Return the background color used for menu.
*
*  @return  Menu background color.
*
**/
/******************************************************************************/
color_t MENU_GetBGndColor( void )
    {
        return BGndColor_Menu;
    }

/*******************************************************************************
*
*                                MENU_Remove
*
*******************************************************************************/
/**
*
*  Remove current menu, clear screen and set pointer "on".
*
**/
/******************************************************************************/
void MENU_Remove( void )
    {
        CurrentMenu = 0;
        DRAW_Clear();
        POINTER_SetMode( POINTER_ON );
        POINTER_SetRectScreen();
        LCD_SetDefaultFont();
        LCD_SetTransparency(0);
/*#if TOUCHSCREEN_AVAIL
        CharMagniCoeff_List = 1;
#endif*/
    }

/*******************************************************************************
*
*                                MENU_Set
*
*******************************************************************************/
/**
*
*  Display provided menu.
*
*  @param[in]  mptr A pointer to the menu to display.
*
**/
/******************************************************************************/
void MENU_Set( tMenu * mptr )
    {
        int lg;
        int lg_max     = 0;
        int i;
        int n          = mptr->NbItems;
        int nlines     = n;

#if TOUCHSCREEN_AVAIL

        // Menu management by list
        if ( CurrentToolbar == &DefaultToolbar )
        {
            TOOLBAR_ChangeButton( 0, 0, 0 ); // Remove the 'config' button
            TOUCHSCR_SetMode( TS_NORMAL );
        }

        // Primer menu management by LIST
        ListMenu.fdispTitle = mptr->fdispTitle;
        ListMenu.Title = mptr->Title;
        ListMenu.NbItems = mptr->NbItems;
        ListMenu.NbDisp = mptr->NbItems;
        ListMenu.XPos = mptr->XPos;
        ListMenu.YPos = mptr->YPos;
        ListMenu.FirstDisplayItem = 0;

        for ( i = 0; i < ListMenu.NbItems; i++ )
        {
            ListMenu.Items[i].Text = ( char* )mptr->Items[i].Text;
        }

        if ( (ListMenu.XPos + ListMenu.YPos) == 0 )
            LIST_Set(( tList* ) &ListMenu, 0, 0, TRUE );
        else
            LIST_Set(( tList* ) &ListMenu, ListMenu.XPos, ListMenu.YPos, FALSE );

#endif  // Touchscreen     

        CurrentMenu = mptr;
        OldCommand = CurrentCommand;

        MENU_ClearCurrentCommand();
        POINTER_SetMode( POINTER_MENU );

#if !TOUCHSCREEN_AVAIL

        // Old menu management
        for ( i = 0; i < n; i++ )
        {
            lg = my_strlen(( char* )mptr->Items[i].Text );

            if ( lg > lg_max )
            {
                lg_max = lg;
            }
        }

        if ( mptr->fdispTitle )
        {
            lg = my_strlen(( char* )mptr->Title );

            if ( lg > lg_max )
            {
                lg_max = lg;
            }

            nlines++;
        }

#if !EXT_FONT    
        Menu_Font = (MENU_BigFont ? 1 : 0);
#endif
        LCD_ChangeFont(Menu_Font);
        mptr->LgMax = lg_max;
        mptr->XSize = (lg_max * Char_Width * CharMagniCoeff) + Char_Width;
        mptr->YSize = (nlines * Char_Height * CharMagniCoeff) + Char_Width;
        mptr->XPos  = ( Screen_Width  - mptr->XSize ) / 2;
        mptr->YPos  = ( Screen_Height - mptr->YSize ) / 2;

        //
        LCD_FillRect_Circle( mptr->XPos, mptr->YPos, mptr->XSize, mptr->YSize, BGndColor_Menu );
        LCD_DrawRect( mptr->XPos, mptr->YPos, mptr->XSize, mptr->YSize, 0 );

        // Display title.
        if ( mptr->fdispTitle )
        {
            RefreshItem( -1, NORMAL_TEXT );
        }

        // Set variables used for filtering/stability.
        CurSel = mptr->SelectedItem;
        Oldsel = CurSel;
        Newsel = CurSel;

        // Display the list of items.
        for ( i = 0 ; i < n ; i++ )
        {
            RefreshItem( i, ( i == mptr->SelectedItem ) ? INVERTED_TEXT : NORMAL_TEXT );
        }

        // No time display while a menu is active.
        fDisplayTime = 0;

        DoubleClickCounter_Menu = MEMS_Info.DoubleClick;
    
        LCD_SetDefaultFont();

#endif // if !TOUCHSCREEN_AVAIL

        BUTTON_SetMode( BUTTON_ONOFF_FORMAIN );
    }

/*******************************************************************************
*
*                                MENU_Question
*
*******************************************************************************/
/**
*
*  Dedicated menu for ask question and yes/no responses.
*
*  @param[in]     str      A pointer to the string containing the question.
*  @param[out]    answer   1 for yes,  0 for no.
*
**/
/********************************************************************************/
void MENU_Question( const u8 * str, bool * answer )
    {
        QuestionMenu.Title   = str;
        CurrentAnswer        = answer;

        MENU_Set( &QuestionMenu );
    }

/***********************************************************************************
*
*                                MENU_ClearCurrentCommand
*
************************************************************************************/
/**
*
*  Set CurrentCommand to 0.
*
**/
/********************************************************************************/
void MENU_ClearCurrentCommand( void )
    {
        CurrentCommand = 0;
    }

/***********************************************************************************
*
*                                MENU_ClearCurrentMenu
*
************************************************************************************/
/**
*
*  Set CurrentMenu to 0
*
**/
/********************************************************************************/
void MENU_ClearCurrentMenu( void )
    {
        CurrentMenu = 0;
    }

/***********************************************************************************
*
*                                MENU_Print
*
************************************************************************************/
/**
*
*  Display a popup menu with a string.
*
*  @param[in]  str The string to display.
*
**/
/********************************************************************************/
void MENU_Print( const u8 * str )
    {
        PrintMenu.Title = str;

        MENU_Set( &PrintMenu );
    }

/***********************************************************************************
*
*                                MENU_SetAppliDivider
*
************************************************************************************/
/**
*
*  Set new value of the call time application divider.
*  Upon each systick, CircleOS will have the opportunity to call your application
*  or not, depending on this divider. If divider = 1, your application will be called
*  upon each systick interrupt. If divider == 2 your application will be called only
*  every other systick occurrence.
*
*  @param[in]  divider : The new value of the divider.
*
**/
/********************************************************************************/
void MENU_SetAppliDivider( divider_t divider )
    {
        Appli_Divider = divider;
    }

/***********************************************************************************
*
*                                MENU_RestoreAppliDivider
*
************************************************************************************/
/**
*
*  Restore the default divider (MENU) for the application divider
*
**/
/********************************************************************************/
void MENU_RestoreAppliDivider( void )
    {
        Appli_Divider = MENU_DIVIDER;
    }
