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
#define MEMS_RES 12
#define TILT_OFFS_Y 5

/* Private functions ---------------------------------------------------------*/
static enum MENU_code MsgVersion(void);

/* Public variables ----------------------------------------------------------*/
const char Application_Name[8+1] = {"My App"};      // Max 8 characters

const u8 *Melody = "GameOver:d=4,o=5,b=210:"
                           "g,f,e\ ";


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
    DRAW_SetTextColor(RGB_BLUE);
    DRAW_DisplayStringWithMode( 0, 5, "Pozycja Nienar", ALL_SCREEN, NORMAL_TEXT, CENTER);
    DRAW_SetDefaultColor();
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
   
   s16 x = MEMS_GetInfo()->OutX_F4 >> 2;
   s16 y = MEMS_GetInfo()->OutY_F4 >> 2;
    
    char tabs[3];
    char dwaps[3];

    DRAW_SetTextColor(RGB_BLUE);
    s16 xx=x;
    if(xx<0) xx=-1*xx;
    xx=xx*90/1000;
    s16 yy=y;
    if(yy<0) yy=-1*yy;
    yy=yy*90/1000;
    UTIL_uint2str(&tabs,xx,3,1);
    UTIL_uint2str(&dwaps,yy,3,1);
    DRAW_DisplayString(30,60,"x:",2);
    if(x<0) DRAW_DisplayString(45,60,"-",1);
        else DRAW_DisplayString(45,60," ",1);
    DRAW_DisplayString(50,60,tabs,sizeof(tabs));
    DRAW_DisplayString(30,40,"y:",2);
    if(y<0) DRAW_DisplayString(45,40,"-",1);
        else DRAW_DisplayString(45,40," ",1);
    DRAW_DisplayString(50,40,dwaps,sizeof(dwaps));
    DRAW_SetDefaultColor();
        
        if((xx>30)||(yy>30)){
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
