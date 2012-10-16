/********************** (C) COPYRIGHT 2007-2010 RAISONANCE ********************
*
* File Name          :  Application.c
* Description        :  Circle_App CircleOS application template.
*
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle_api.h"

/* Private defines -----------------------------------------------------------*/

// The following should be the minimal CircleOS version needed by your application
#define NEEDEDVERSION "V 4.0"

/* Private functions ---------------------------------------------------------*/
static enum MENU_code MsgVersion(void);

/* Public variables ----------------------------------------------------------*/
const char Application_Name[8+1] = {"WW"};      // Max 8 characters
const u8 *Melody = "Haha:d=4,o=5,b=310: e,f,g\ ";

/*******************************************************************************
* Function Name  : Application_Ini
* Description    : Initialization function of Circle_App. This function will
*                  be called only once by CircleOS.
* Input          : None
* Return         : MENU_CONTINUE_COMMAND
*******************************************************************************/
enum MENU_code Application_Ini(void)
    {
    // Ensure that the current OS version is recent enough
    if(strcmp(UTIL_GetVersion(), NEEDEDVERSION) < 0)
        {
        return MsgVersion();
        }
    
    // This application manages all the screen. 
    // If you don't reset the offset on EvoPrimer, the screen will be reduce
    // to 128x128 pixels for Primer2 compatibility
    LCD_SetOffset(OFFSET_OFF);

    // TODO: Write your application initialization function here.
#if 1
    DRAW_DisplayStringWithMode( 0, 10, "Aplikacja Witold", ALL_SCREEN, NORMAL_TEXT, CENTER);
#endif
    
    // Your can set the frequency of the call by the system.
    MENU_SetAppliDivider(1000);

    return MENU_CONTINUE_COMMAND;
    }

/*******************************************************************************
* Function Name  : Application_Handler
* Description    : Management of the Circle_App. This function will be called
*                  every millisecond by CircleOS while it returns MENU_CONTINUE.
* Input          : None
* Return         : MENU_CONTINUE
*******************************************************************************/
enum MENU_code Application_Handler(void)
    {
    // This routine will get called repeatedly by CircleOS, until we
    // return MENU_LEAVE

    // TODO: Write your application handling here.

               color_t LineColor = RGB_BLUE;
    
        
            
            s16 x = MEMS_GetInfo()->OutX_F4 >> 2;
            s16 y = MEMS_GetInfo()->OutY_F4 >> 2;
            
            DRAW_SetTextColor(RGB_RED);
 
            if(x<0)
                {                
                x=-1*x;
                }
            
            if(y<0) 
                {
                y=-1*y;
                }
            
            x=x*90/1000;            
            y=y*90/1000;
            
            char X[5];
            char Y[5];
            UTIL_uint2str(&X,x,4,1);
            UTIL_uint2str(&Y,y,4,1);

            DRAW_DisplayString(30, 80, "x: ", sizeof(4));
            DRAW_DisplayString(60, 80, X, sizeof(X));
            DRAW_DisplayString(30, 60, "y: ", sizeof(4));
            DRAW_DisplayString(60, 60, Y, sizeof(Y));
                
            if((x>30)||(y>30))
            {
            BUZZER_SetMode(BUZZER_ON);
            BUZZER_PlayMusic(Melody);
            }
            
        
    // If the button is pressed, the application is exited
    if (BUTTON_GetState() == BUTTON_PUSHED)
        {
        BUTTON_WaitForRelease();
        return MENU_Quit();
        }

    return MENU_CONTINUE;   // Returning MENU_LEAVE will quit to CircleOS
    }

/*******************************************************************************
* Function Name  : MsgVersion
* Description    : Displays the current CircleOS version and the version needed
*                  exit to main menu after 4 seconds
* Input          : None
* Return         : MENU_REFRESH
*******************************************************************************/
static enum MENU_code MsgVersion(void)
    {
    u8 hh, mm, ss, ss2;

    DRAW_DisplayString( 5,  60, "CircleOS", 17 );
    DRAW_DisplayString( 80, 60, UTIL_GetVersion(), 6 );
    DRAW_DisplayString( 5,  34, NEEDEDVERSION, 6 );
    DRAW_DisplayString( 50, 34, " required", 12 );

    RTC_GetTime( &hh, &mm, &ss );
    ss = ss + 4;
    ss = ss % 60;

    do
        {
        RTC_GetTime( &hh, &mm, &ss2 );
        }
    while ( ss2 != ss );           // do while < 4 seconds

    DRAW_Clear();
    return MENU_REFRESH;
    }
