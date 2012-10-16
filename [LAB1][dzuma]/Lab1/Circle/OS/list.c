/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     list.c
* @brief    General functions to handle list with touchscreen.
* @author   YRT / FL
* @date     11/2008
* @version  4.0  Add Open4 Primer
* @date     10/2009
*
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private defines -----------------------------------------------------------*/
#define MAXLOCKMEMS  200
#define ANGLEPAUSE                  -100
#define MIN_ANGLE_FOR_SHIFT_UP      (ANGLEPAUSE+CurrentAngleStart + 150)
#define MIN_ANGLE_FOR_SHIFT_DOWN    (ANGLEPAUSE-CurrentAngleStart - 150)
#define TIME_FOR_TOUCH_LIST  (2000)
#define LIST_DIVIDER 10
#define DELAY_BEFORE_DBCLK 80

/* define for joystick management*/
#define JOY_MAXBTIME 20
#define JOY_HIGH_SPEED 5
#define JOY_LOW_SPEED 1

#define FIRST_VISIBLE      ( CurrentList->FirstDisplayItem - ( (nb_move<0) ? 1 :  0 ) )

/* Extern variables ----------------------------------------------------------*/
extern color_t bmpTmp[LCD_DMA_SIZE];        /* 16 bits words : buffer to scroll display */

/* Private variables ---------------------------------------------------------*/
//u16     CharMagniCoeff_List = 1;
s32     NewSelList = 0;                 /*!< Match with the immediate position.  */
s32     DisplaySelectedItem = 0;
s32     CurOrientList = V12;            /* Current orientation of the screen*/
s32     OldOrientList = -1;
bool    move_old_Pressed = FALSE;
s32     nb_move = 0;
s32     Max_X_List, Min_X_List;
s32     Max_Y_List, Min_Y_List;
u16     ListCharWidth;
s32     ListCharHeight;
static  s32 ListTimePressed = 0;
static  divider_t ListCptDivider = 0;

/* Rq: 3 bytes by pixel but transfer in 16 bits bus format*/
tList* CurrentList = 0;

/* Variables for joystick*/
static enum JOYSTICK_state  JoyPos;
static u16                  JoyDelay  = 0;
static u16                  JoyInc    = 1;


/* Private function prototypes -----------------------------------------------*/
void LIST_RefreshItem( index_t sel, bool isInverted );
s32  LIST_GetNewSelectedItem( void );
s32  LIST_DetectMove( void );
void StoreStringWithMode( u16* buffer, const u8* ptr, u8 len, u8 offset, u8 nblines );
void StoreChar( u16* buffer, u8* bmp, u8 nblines, u8 offset, color_t textColor, color_t bGndColor, mag_t CharMagniCoeff );
void SearchNewFirstItem( void );
void InitListDMA( void );
void LIST_LCD_RectRead( coord_t x, coord_t y, coord_t width, coord_t height );
void LIST_DRAW_SetImage( coord_t x, coord_t y, coord_t width, coord_t height );
void LIST_StoreString( u16* buffer, const u8* ptr, u8 len, u8 offset, u8 nblines, u16 textColor, u16 bGndColor, u16 CharMagniCoeff );
void LIST_StoreChar( u16* buffer, u8* bmp, u8 nblines, u8 offset, u16 textColor, u16 bGndColor, u16 CharMagniCoeff );

/* Public function prototype -------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

extern u32 DoubleClickCounter_Menu;
extern color_t title_BGndColor;
extern color_t title_TextColor;
extern color_t selected_BGndColor;
extern color_t selected_TextColor;
extern s16  CurrentAngleStart;

/*******************************************************************************
*
*                                SearchNewFirstItem
*
*******************************************************************************/
/**
*  Search and update the index of the first item to display
*
*
**/
/******************************************************************************/
NODEBUG2 void SearchNewFirstItem( void )
{
    s32 nb_char_move;

    /* Calculate the new first item*/
    nb_char_move = ( nb_move * DeltaY )  / ListCharHeight;
    if ((( nb_char_move > 0 ) && ( CurrentList->FirstDisplayItem < ( CurrentList->NbItems - CurrentList->NbDisp ) ) )
            || (( nb_char_move < 0 ) && ( CurrentList->FirstDisplayItem > 0 ) ) )
    {
        CurrentList->FirstDisplayItem  += nb_char_move;
        DisplaySelectedItem -= nb_char_move;
        nb_move -= ( nb_char_move * ListCharHeight / DeltaY );
        ListTimePressed = 0;
    }
}

/*******************************************************************************
*
*                                LIST_GetSelectedItem
*
*******************************************************************************/
/**
*  See if and which item has been selected of the list displayed
*
*  @return  SelectedItem = index of the selected item
*                          on the screen : 0 to MAX_DISP_LIST
*
**/
/******************************************************************************/
#if TOUCHSCREEN_AVAIL
NODEBUG2 s32 LIST_GetNewSelectedItem( void )
{
    u16 X, Y;
    s32 SelectedItem = -1;
    static bool fJoystickHasInput_old = 0;
    bool fIsPressed = TOUCHSCR_IsPressed();
    static s32 MemsLockCounter = MAXLOCKMEMS ;
    bool fJoystickHasInput;


    /* See if a hit on the touchscreen has been done*/
    if ( fIsPressed )
    {
        /* Get the position*/
        X = TOUCHSCR_GetPosX();
        Y = TOUCHSCR_GetPosY();
    
        /* If touch is on the screen (not on toolbar)*/
        if (( X < Screen_Width ) && ( Y < Screen_Height ) )
        {
            /* We capture the touch, even it's not into the list*/
            if (( Y < Min_Y_List )  || ( Y > Max_Y_List ) )
                Y =  Max_Y_List;

            /* An item is just selected*/
            SelectedItem = ( Max_Y_List - Y + (nb_move * DeltaY)) / ListCharHeight ;
            if ( SelectedItem >= CurrentList->NbDisp )
            {
                SelectedItem = CurrentList->NbDisp - ( nb_move ? 0 : 1 );
            }
            if ( SelectedItem < 0 )
            {
                SelectedItem = 0;
            }
            if ( SelectedItem != DisplaySelectedItem )
            {
                ListTimePressed  = 0;
            }
        }
    }
    fJoystickHasInput = FALSE;

    /*-- Option management of the pointer by the joystick  */
    if ( JoystickAsInput && ( SelectedItem == -1 ) )
    {
        JoyPos = JOYSTICK_GetState();

        if ( JoyPos != JOYSTICK_RELEASED )
        {
            fJoystickHasInput = TRUE;
            JoyDelay++;
        }
        else
        {
            JoyDelay = 0;
        }

        if (( fJoystickHasInput_old == 0 ) || ( JoyDelay >= WEIGHTED_TIME( 3 * JOY_MAXBTIME ) ) )
        {
            JoyDelay = 0;

            if (( JoyPos == JOYSTICK_DOWN ) || ( JoyPos == JOYSTICK_LEFT_DOWN ) || ( JoyPos == JOYSTICK_RIGHT_DOWN ) )
            {

                if (( DisplaySelectedItem < CurrentList->NbDisp ) && ( CurrentList->SelectedItem < CurrentList->NbItems - 1 ) )
                    SelectedItem = DisplaySelectedItem + 1;
            }

            if (( JoyPos == JOYSTICK_UP ) || ( JoyPos == JOYSTICK_LEFT_UP ) || ( JoyPos == JOYSTICK_RIGHT_UP ) )
            {
                if ( DisplaySelectedItem > 0 )
                    SelectedItem = DisplaySelectedItem - 1;

            }

        }
        fJoystickHasInput_old = fJoystickHasInput;
    }

    /*-- Option management of the pointer by the mems  */
    if ( SelectedItem != -1 )
    {
        MemsLockCounter = WEIGHTED_TIME( 2 * MAXLOCKMEMS );
    }

    if ( MemsLockCounter )
    {
        MemsLockCounter--;
    }
    else
    {
        if ( MemsAsInput && ( SelectedItem == -1 ) && ( JoyPos == JOYSTICK_RELEASED ) && ( fIsPressed == 0 ) )
        {

            if (( DisplaySelectedItem < CurrentList->NbDisp ) && ( CurrentList->SelectedItem < CurrentList->NbItems - 1 )
                    && ( MEMS_Info.RELATIVE_Y < MIN_ANGLE_FOR_SHIFT_DOWN ) )
            {
                SelectedItem = DisplaySelectedItem + 1;
            }
            if (( DisplaySelectedItem > 0 ) && ( MEMS_Info.RELATIVE_Y > MIN_ANGLE_FOR_SHIFT_UP ) )
            {
                SelectedItem = DisplaySelectedItem - 1;
            }
            if ( SelectedItem != -1 )
            {
                MemsLockCounter = WEIGHTED_TIME( MAXLOCKMEMS / 3 );
            }
        }
    }
    return SelectedItem;
}
#endif //TOUCHSCREEN_AVAIL


/*******************************************************************************
*
*                                LIST_DetectMove
*
*******************************************************************************/
/**
*  Detect the direction of the move on the touchscreen
*
*  @return  dir = direction of the touch move :
*                    0 : no move
*                    x delta Y : go to the end of the list
*                   -x delta Y : go to the top of the list
*
**/
/******************************************************************************/
#if TOUCHSCREEN_AVAIL
NODEBUG2 s32 LIST_DetectMove( void )
{
    static s32 move_old_Y = 0;
    s32 dir = 0;
    s32 move_cur_Y, X;
    static s32 MemsLockCounter = MAXLOCKMEMS ;
    bool fJoystickHasInput;


    /* See if a hit on the touchscreen is always done*/
    if ( TOUCHSCR_IsPressed() )
    {
        /* Get the new position*/
        X = TOUCHSCR_GetPosX();
        move_cur_Y = TOUCHSCR_GetPosY();

        if (( X > Min_X_List ) && ( X < Max_X_List ) )
        {
            if ( move_old_Pressed == FALSE )
            {
                move_old_Y = move_cur_Y;
            }
            else
            {
                dir = ( move_cur_Y - move_old_Y ) / DeltaY;
                if ( dir >= 1 )
                {
                    dir = 1;
                    move_old_Y += DeltaY;
                }
                else if ( dir <= -1 )
                {
                    dir = -1;
                    move_old_Y -= DeltaY;
                }
            }
            move_old_Pressed = TRUE;
        }
    }
    else
    {
        /* Touch release*/
        move_old_Pressed = FALSE;
        dir = 0;
    }

    /*-- Option management of the pointer by the joystick  */
    fJoystickHasInput = FALSE;
    if ( JoystickAsInput )
    {
        JoyPos = JOYSTICK_GetState();
        if (( JoyPos == JOYSTICK_UP ) || ( JoyPos == JOYSTICK_LEFT_UP ) || ( JoyPos == JOYSTICK_RIGHT_UP ) )
        {
            if ( DisplaySelectedItem <= 0 )
                dir = - JoyInc ;
        }
        if (( JoyPos == JOYSTICK_DOWN ) || ( JoyPos == JOYSTICK_LEFT_DOWN ) || ( JoyPos == JOYSTICK_RIGHT_DOWN ) )
        {
            if ( DisplaySelectedItem >= CurrentList->NbDisp )
                dir = + JoyInc ;
        }


        if ( JoyPos != JOYSTICK_RELEASED )
        {
            fJoystickHasInput = TRUE;
            JoyDelay++;
        }
        else
        {
            JoyDelay = 0;
        }

        if ( JoyDelay >= JOY_MAXBTIME )
            JoyInc = JOY_HIGH_SPEED;
        else
            JoyInc = JOY_LOW_SPEED;
    }

    /*-- Option management of the pointer by the mems  */
    if ( dir )
    {
        MemsLockCounter = WEIGHTED_TIME( MAXLOCKMEMS );
    }

    if ( MemsLockCounter )
    {
        MemsLockCounter--;
    }
    else
    {
        if ( MemsAsInput && ( dir == 0 ) && ( JoyPos == JOYSTICK_RELEASED ) && ( move_old_Pressed == 0 ) )
        {
            extern s16  CurrentAngleStart ;

            if ((( DisplaySelectedItem >= CurrentList->NbDisp ) && ( MEMS_Info.RELATIVE_Y < MIN_ANGLE_FOR_SHIFT_DOWN ) )
                    || (( DisplaySelectedItem <= 0 ) && ( MEMS_Info.RELATIVE_Y > MIN_ANGLE_FOR_SHIFT_UP ) ) )
            {
                dir = -MEMS_Info.RELATIVE_Y ;
            }
        }
    }
    return dir;
}
#endif //TOUCHSCREEN_AVAIL

/*******************************************************************************
*
*                                LIST_RefreshItem
*
*******************************************************************************/
/**
*  Display the text of the command.
*
*  @param[in]  sel         The index of the item in the visible part of the list.
*  @param[in]  isInverted  When non null, text is displayed inverted.
*
**/
/******************************************************************************/
NODEBUG2 void LIST_RefreshItem( index_t sel, bool isInverted )
{
    s32 lg = CurrentList->LgMax;
    s32 save_BGndColor      = DRAW_GetBGndColor();
    s32 save_TextColor      = DRAW_GetTextColor();
    s32 offs;

    /* Calculate the index into the original list*/
    s32 index = sel + CurrentList->FirstDisplayItem; /*FIRST_VISIBLE ; //FL090103 */

    /* If not title, do not draw the title*/
    if (( !CurrentList->fdispTitle ) && ( sel == -1 ) )
        return;

    /* Set menu DRAW params*/
    DRAW_SetBGndColor(( sel == -1 ) ? title_BGndColor : BGndColor_Menu );
    DRAW_SetTextColor(( sel == -1 ) ? title_TextColor : TextColor_Menu );

    offs = ( sel == -1 ) ? 0 : ( nb_move * DeltaY );

    /* Standard display*/
    if ( !isInverted )
    {
        if ( sel != -1 ) /* Title is always complete*/
            LCD_DrawCharSetFilter( Min_X_List, Max_X_List, Min_Y_List, Max_Y_List );

        DRAW_DisplayString( Min_X_List,
                            CurrentList->YPos - (( sel + 2 ) * ListCharHeight ) - ListCharHeight / 4  + offs,
                            ( sel == -1 ) ? ( u8* )CurrentList->Title : ( u8* )( CurrentList->Items[index].Text ),
                            lg );

        if ( sel != -1 ) /* Title is always complete*/
        {
            // Remove filter
            LCD_DrawCharSetFilter( 0, Screen_Width, 0, Screen_Height );
        }
    }
    else
    {
        /* Inverted display.*/
        DRAW_SetBGndColor(( sel == -1 ) ? title_BGndColor : selected_BGndColor );
        DRAW_SetTextColor(( sel == -1 ) ? title_TextColor : selected_TextColor );
        if ( sel != -1 ) /* Title is always complete*/
            LCD_DrawCharSetFilter( Min_X_List, Max_X_List, Min_Y_List, Max_Y_List );

        DRAW_DisplayString( Min_X_List,
                            CurrentList->YPos - (( sel + 2 ) * ListCharHeight ) - ListCharHeight / 4 + offs,
                            ( sel == -1 ) ? ( u8* ) CurrentList->Title : ( u8* )( CurrentList->Items[index].Text ),
                            lg );

        if ( sel != -1 ) /*Title is always complete*/
        {
            // Remove filter
            LCD_DrawCharSetFilter( 0, Screen_Width, 0, Screen_Height );
        }
    }

    /* Restore previous DRAW params.*/
    DRAW_SetBGndColor( save_BGndColor );
    DRAW_SetTextColor( save_TextColor );
}


/*******************************************************************************
*
*                                LIST_StoreString
*
*******************************************************************************/
/**
*
*  This function is used to store x lines of a 17 char max string of
*  characters into memory for further display.
*
*  @param[in]  buffer   Pointer to the buffer where to store the string into.
*  @param[in]  ptr      Pointer to string to display.
*  @param[in]  len      String size.
*  @param[in]  offset   Number of the first pixels line of the string to store.
*  @param[in]  nblines  Number of pixels lines to store.
*
*
**/
/******************************************************************************/
NODEBUG2 void LIST_StoreString( u16* buffer, const u8* ptr, u8 len, u8 offset, u8 nblines, u16 textColor, u16 bGndColor, u16 CharMagniCoeff )
{
    u8 c, car = 0,Char_By_Line;

    Char_By_Line = Screen_Width / (Char_Width * CharMagniCoeff ); /* Calculate the maximum characters by line */

    /* Up to x characters*/
    if ( len > Char_By_Line )
        len = Char_By_Line;


    /* Store each character */
    for ( car = 0 ; car < len ; car++ )
    {
        c = *ptr;
        if ( c )
        {
            ptr++;      /* Point to the next character */
        }
        else
        {
            c = ' ';    /* fill with space when len exceeds strlen(ptr) */
        }

        /* Store one character */
        /*        LIST_StoreChar((u16*) buffer, (u8*)&AsciiDotsTable[((c-32) * 14)], nblines, offset, textColor,  bGndColor, CharMagniCoeff ); // YRT20090304*/
        LIST_StoreChar(( u16* ) buffer, ( u8* )&CurrentFont[(( c-32 ) * (2 * Char_Width) )], nblines / CharMagniCoeff, offset, textColor,  bGndColor, CharMagniCoeff );

        /* next buffer word*/
        buffer += ( nblines * Char_Width * CharMagniCoeff); /* height (up to 14 pixels) *  width (7 pixels)*/
    }
}


/*******************************************************************************
*
*                                LIST_StoreChar
*
*******************************************************************************/
/**
*
*  Store into memory the provided ASCII character with the provided
*  text and background colors and with the provided magnify coefficient.
*  Note : 2 bytes by pixel
*
*  @param[in]  buffer         Pointer to the buffer where to store the string into.
*  @param[in]  bmp            The ASCII code of the character to display.
*                             @n Ascii must be higher than 31 and lower than 255.
*  @param[in]  offset         Number of the first pixels line of the string to store.
*  @param[in]  nblines        Number of pixels lines to store.
*  @param[in]  textColor      The color used to draw the character.
*  @param[in]  bGndColor      The background color of the drawn character.
*  @param[in]  CharMagniCoeff The magnify coefficient used to draw the character.
*
**/
/******************************************************************************/
NODEBUG2 void LIST_StoreChar( u16* buffer, u8* bmp, u8 nblines, u8 offset, u16 textColor, u16 bGndColor, u16 CharMagniCoeff )
{
    u8 i = 0;
    u16 mask;
    u16 k1, k2, k3, k4;
    u16 matrix;

    /* First line to store*/
    k3 = 0x8000;
    for ( i = 0 ; i < offset; i++ )
        k3 >>= 1;

    /* End line to store*/
    k4 = k3;
    for ( i = 0 ; i < nblines; i++ )
        k4 >>= 1;

    /* For each pixel column of the character*/
    for ( i = 0; i < Char_Width ; i++ )
    {
        /* Concatenation of even and odd lines of the matrix*/
        matrix = ( bmp[2*i] << 8 ) | bmp[2*i+1];

        for ( k1 = 0; k1 < CharMagniCoeff; k1++ )
        {
            /* Scan all bits from bottom to top of character*/
            for ( mask = k3; mask > k4 ; mask >>= 1 )
            {
                for ( k2 = 0; k2 < CharMagniCoeff; k2++ )
                {
                    *( buffer++ ) = (( matrix & mask ) ? textColor : bGndColor ) ;
                }
            }
        }
    }
}

/// @endcond

/* Public functions ----------------------------------------------------------*/

/*******************************************************************************
*
*                                LIST_Set
*
*******************************************************************************/
/**
*
*  Display provided list.
*
*  @param[in]  lptr A pointer to the list to display.
*  @param[in]  posX Position on X axis.
*  @param[in]  posY Position on Y axis.
*  @param[in]  center : = 1 if the list has to be center on the screen.
*
* Note : posX, posY are the coordinates of the top left corner of the list
*
**/
/******************************************************************************/
void LIST_Set( tList* lptr, coord_t posX, coord_t posY, bool center )
{
    s32 i, lg;
    s32 lg_max = 0;
    s32 XSize;
    s32 YSize;

    s32 save_BGndColor      = DRAW_GetBGndColor();
    s32 save_TextColor      = DRAW_GetTextColor();

    /* Initialize colors*/
    DRAW_SetBGndColor( BGndColor_Menu );
    DRAW_SetTextColor( TextColor_Menu );

    /* Lock RotateScreen*/
    /*    PreviousRotateScreen = LCD_GetRotateScreen(); // YRT20090304*/
    /*    LCD_SetRotateScreen(0);                       */
    POINTER_SetMode( POINTER_OFF );     /* YRT20090325 fix bug square on the list*/

    /* Affect the current list to the newlist */
    CurrentList = lptr;

    /* Set variables used for filtering/stability.*/
    DisplaySelectedItem = 0;
    CurrentList->SelectedItem = 0;
    CurrentList->FirstDisplayItem = 0; /*first visible item*/
    nb_move = 0;
    ListTimePressed = 0;


    if ( CurrentList->NbItems < CurrentList->NbDisp )
        CurrentList->NbDisp = CurrentList->NbItems;

    /* Calculate the max length of the lines*/
    for ( i = 0; i < CurrentList->NbItems; i++ )
    {
        lg = my_strlen(( u8* )CurrentList->Items[i].Text );

        if ( lg > lg_max )
            lg_max = lg;
    }
    if ( CurrentList->fdispTitle )
    {
        lg = my_strlen(( u8* )CurrentList->Title );

        if ( lg > lg_max )
        {
            lg_max = lg;
        }
    }

    CurrentList->LgMax = lg_max;

    /* Calculate the size of the list and it's position*/
    LCD_ChangeFont(Menu_Font);
    ListCharWidth = Char_Width * DRAW_GetCharMagniCoeff();
    ListCharHeight = Char_Height * DRAW_GetCharMagniCoeff();

    CurrentList->XSize = ( lg_max + 1 ) * ListCharWidth;
    CurrentList->YSize = (( CurrentList->NbDisp + 1 ) * ListCharHeight ) + ListCharHeight / 2;
    if ( center )
    {
        CurrentList->XPos  = ( Screen_Width  - CurrentList->XSize ) / 2;
        CurrentList->YPos  = (( Screen_Height - CurrentList->YSize ) / 2 ) + CurrentList->YSize;
    }
    else
    {
        CurrentList->XPos  = posX;
        CurrentList->YPos  = posY;
    }

    /* Calculate the active zone of the list (the title is excluded)*/
    Max_Y_List = CurrentList->YPos - ListCharHeight - ListCharHeight / 4;
    Min_Y_List = Max_Y_List - CurrentList->NbDisp * ListCharHeight ;
    XSize  = ( lg_max ) * ListCharWidth;
    YSize  = ( Max_Y_List - Min_Y_List ) + ( ListCharHeight );
    Min_X_List = ( CurrentList->XPos ) + ( ListCharWidth / 2 );
    Max_X_List = Min_X_List + XSize;

    /* Clear the screen*/
    LCD_FillRect_Circle( CurrentList->XPos, CurrentList->YPos - CurrentList->YSize, CurrentList->XSize, CurrentList->YSize, BGndColor_Menu );
    LCD_DrawRect( CurrentList->XPos, CurrentList->YPos - CurrentList->YSize, CurrentList->XSize, CurrentList->YSize, 0 );

    /* Display title.*/
    if ( CurrentList->fdispTitle )
    {
        LIST_RefreshItem( -1, NORMAL_TEXT );
    }

    /* See if empty list*/
    if ( CurrentList->NbItems != 0 )
    {
        /* Display the list of items.*/
        for ( i = 0 ; i < CurrentList->NbDisp ; i++ )
        {
            LIST_RefreshItem( i, ( i == DisplaySelectedItem ) ? INVERTED_TEXT : NORMAL_TEXT );
        }
    }

    /* No time display while a menu is active.*/
    fDisplayTime = 0;

    /*     BUTTON_SetMode( BUTTON_ONOFF_FORMAIN ); // YRT20080204*/
    /* Rq : the mode is ONOFF if called by an application*/
    /* but it's forced to FORMAIN if called by menu handler*/
    BUTTON_SetMode( BUTTON_ONOFF );
    DoubleClickCounter_Menu = MEMS_Info.DoubleClick;

    /* Initialize DMA for optimize screen scroll*/
    InitListDMA();

    /* Restore previous DRAW params.*/
    DRAW_SetBGndColor( save_BGndColor );
    DRAW_SetTextColor( save_TextColor );

    ListCptDivider = 0;

}

/*******************************************************************************
*
*                                LIST_Manager
*
*******************************************************************************/
/**
*  Management of a list of items , with touchscreen functionnalities
*
*  @return Index of the item selected, -1 if no selection
*
*
**/
/******************************************************************************/
s32 LIST_Manager( void )
{
#if TOUCHSCREEN_AVAIL
    s32 i;
    const u8* ptr;
    u16 x, y, width;
    s32 offset, move = 0;
    u16 txtcolor;
    u16 bgdcolor;

    /* No list to manage, exit*/
    if ( CurrentList == 0 )
        return -1;

    ListCptDivider++;

    x = Min_X_List;
    width = Max_X_List - Min_X_List;
    ListCharWidth = Char_Width * DRAW_GetCharMagniCoeff();
    ListCharHeight = Char_Height * DRAW_GetCharMagniCoeff();
    TOUCHSCR_SetSensibility( 3000 );

    /* Determine the new selection if any*/
    NewSelList = move_old_Pressed ? DisplaySelectedItem : LIST_GetNewSelectedItem();

    if (( NewSelList != -1 ) && ( NewSelList != DisplaySelectedItem ) )
    {
        /* First refresh CurrentMenu->SelectItem (the old selected item)*/
        if (( DisplaySelectedItem >= 0 ) && ( DisplaySelectedItem <= CurrentList->NbDisp ) )
        {
            LIST_RefreshItem( DisplaySelectedItem, NORMAL_TEXT );
        }

        /* Update the selected item*/
        DisplaySelectedItem = NewSelList;
        /*        CurrentList->SelectedItem = DisplaySelectedItem + FIRST_VISIBLE ; //FL090103 +CurrentList->FirstDisplayItem ; // YRT20080226*/
        CurrentList->SelectedItem = DisplaySelectedItem + CurrentList->FirstDisplayItem ;

        /* Then select the new selected item*/
        if (( DisplaySelectedItem >= 0 ) && ( DisplaySelectedItem <= CurrentList->NbDisp ) )
        {
            LIST_RefreshItem( DisplaySelectedItem, INVERTED_TEXT );
        }
    }

    /* See if the orientation of the screen has changed*/
    CurOrientList = LCD_GetScreenOrientation();

    /* Redraw items if needed (if menu called, the redraw is made by the LIST_Set() call)*/
    if (( CurOrientList != OldOrientList )
            && ( CurrentMenu == 0 ) )                   /* YRT200903004*/
    {
        LCD_FillRect_Circle( CurrentList->XPos, CurrentList->YPos - CurrentList->YSize, CurrentList->XSize, CurrentList->YSize, BGndColor_Menu );
        LCD_DrawRect( CurrentList->XPos, CurrentList->YPos - CurrentList->YSize, CurrentList->XSize, CurrentList->YSize, RGB_BLACK );

        /* Redraw the title*/
        LIST_RefreshItem( -1, NORMAL_TEXT );

        /* Redraw the list of items.*/
        for ( i = 0 ; i < CurrentList->NbDisp ; i++ )
        {
            LIST_RefreshItem( i, ( i == DisplaySelectedItem ) ? INVERTED_TEXT : NORMAL_TEXT );
        }
    }
    OldOrientList = CurOrientList;

    /* See if scroll request*/
    move = LIST_DetectMove();

    if ( move != 0 )
    {
        /* Redraw the list of items.*/
        if ( move < 0 )
        {
            /* Move to the bottom of the screen => to the top of the list*/
            if ( CurrentList->FirstDisplayItem > 0 )
            {
                /* Scroll the screen each DeltaY lines to the bottom*/
                for ( y = Min_Y_List + DeltaY; y < Max_Y_List ; y += DeltaY )
                {
                    LIST_LCD_RectRead( x, y, width, DeltaY );
                    LIST_DRAW_SetImage( x, y - DeltaY, width, DeltaY );
                }

                nb_move--;

                /* Store new string into the top cache and show it*/
                if ( CurrentList->FirstDisplayItem > 0 )
                {
                    ptr = ( u8* )( CurrentList->Items[FIRST_VISIBLE/*CurrentList->FirstDisplayItem - 1*/].Text );
                    offset = (( nb_move + 1 ) * DeltaY / DRAW_GetCharMagniCoeff() ) %  Char_Height;
                    offset *= -1;
                    while ( offset < 0 )
                    {
                        offset +=  Char_Height;
                    }
                    txtcolor = (( DisplaySelectedItem + (( nb_move < 0 ) ? 1 :  0 ) ) == 0 ) ? selected_TextColor : TextColor_Menu ;
                    bgdcolor = (( DisplaySelectedItem + (( nb_move < 0 ) ? 1 :  0 ) ) == 0 ) ? selected_BGndColor : BGndColor_Menu ;
                    LIST_StoreString(( u16* )&bmpTmp, ptr, CurrentList->LgMax + 1, offset, DeltaY, txtcolor, bgdcolor, DRAW_GetCharMagniCoeff() );
                    /* Load top cache into the screen*/
                    DRAW_SetImage(( const color_t* ) &bmpTmp, x, Max_Y_List - DeltaY, width, DeltaY );
                }
            }
        }
        else
        {
            /* To the top of the screen => to the bottom of the list*/
            if ( CurrentList->FirstDisplayItem < ( CurrentList->NbItems - CurrentList->NbDisp ) )
            {
                /* Scroll the screen each DeltaY lignes to the top*/
                for ( y = Max_Y_List - ( 2 * DeltaY ) ; y >= Min_Y_List ; y -= DeltaY )
                {
                    LIST_LCD_RectRead( x, y, width, DeltaY );
                    LIST_DRAW_SetImage( x, y + DeltaY, width, DeltaY );
                }
                nb_move++;

                /* Store new string into the bottom cache and show it*/
                if (( CurrentList->FirstDisplayItem + CurrentList->NbDisp ) < CurrentList->NbItems )
                {
                    ptr = ( u8* )( CurrentList->Items[FIRST_VISIBLE + CurrentList->NbDisp].Text );
                    offset = ( Char_Height - ( nb_move * DeltaY / DRAW_GetCharMagniCoeff() ) ) % Char_Height;
                    if ( offset < 0 )
                        offset = Char_Height - offset;

                    txtcolor = ( DisplaySelectedItem == ( CurrentList->NbDisp - (( nb_move <= 0 ) ? 1 : 0 ) ) ) ? selected_TextColor : TextColor_Menu ;
                    bgdcolor = ( DisplaySelectedItem == ( CurrentList->NbDisp - (( nb_move <= 0 ) ? 1 : 0 ) ) ) ? selected_BGndColor : BGndColor_Menu ;

                    LIST_StoreString(( u16* )&bmpTmp, ptr, CurrentList->LgMax + 1, offset, DeltaY, txtcolor, bgdcolor, DRAW_GetCharMagniCoeff() );

                    /* Load bottom cache into the screen*/
                    DRAW_SetImage(( const color_t* ) &bmpTmp, x, Min_Y_List, width, DeltaY );
                }
            }
        } /* else move< 0*/
        SearchNewFirstItem();

    } /* end if move!=0*/

#if 0 /* For debugging purpose, display the state of the list in the title bar*/
    static u8 buffer[15] = "F10 M-XX S02";
    UTIL_uint2str( buffer + 1, FIRST_VISIBLE /* CurrentList->FirstDisplayItem */ + 1, 2, 1 );
    buffer[3] = ' ';

    if ( nb_move < 0 )
    {
        buffer[5] = '-';
        UTIL_uint2str( buffer + 6, -nb_move, 2, 1 );
    }
    else
    {
        buffer[5] = '+';
        UTIL_uint2str( buffer + 6, +nb_move, 2, 1 );
    }
    buffer[8] = ' ';
    /*        UTIL_uint2str( buffer+10, DisplaySelectedItem + CurrentList->FirstDisplayItem +1, 2, 1 ); // YRT090107*/
    UTIL_uint2str( buffer + 10, CurrentList->SelectedItem, 2, 1 );
    CurrentList->Title = buffer;
    LIST_RefreshItem( -1, NORMAL_TEXT );
#endif

    /* Inhibition of the mems doubleclick */
    if ( ListCptDivider < DELAY_BEFORE_DBCLK )
        DoubleClickCounter_Menu = MEMS_Info.DoubleClick;

    /* Check item selection validation */
    if (( BUTTON_GetState() == BUTTON_PUSHED_FORMAIN )         /* YRT20090226*/
            || ( BUTTON_GetState() == BUTTON_PUSHED )
            || ( DoubleClickCounter_Menu != MEMS_Info.DoubleClick )
            || ((( ListTimePressed * freqTIM2[SPEED_MEDIUM] ) / freqTIM2[CurrentSpeed] ) > TIME_FOR_TOUCH_LIST ) )
    {
        BUTTON_WaitForRelease();
        DoubleClickCounter_Menu = MEMS_Info.DoubleClick;

        return CurrentList->SelectedItem;
    }

    /* Increment duration of the screen pressed state*/
    if ( !TOUCHSCR_IsPressed() )
    {
        ListTimePressed = 0;
    }
    else
    {
        ListTimePressed  += LIST_DIVIDER;
    }

    return -1;
#endif
}

