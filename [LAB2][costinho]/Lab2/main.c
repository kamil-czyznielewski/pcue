/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     main.c
* @brief    CircleOS main with initialization of the different modules.
* @author   FL
* @date     07/2007
* @note     Doesn't do anything more when the hardware is initialized.
* @date     10/2007
* @version  1.6 Place the NVIC Vector Table in RAM for per-user customization
*
* @version  3.0 Add Primer 2 and ST library v2.0.3
* @date     11/2008 
*
* @version  4.0 Add Open4 Primer
* @date     10/2009 
*
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

int x = 0;
s16 aagg = 0;
const u8 *Melody1 = "Tetris:d=4,o=5,b=210:"
                           "e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,"
                           "8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,"
                           "8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a\ ";

/* Private variables ---------------------------------------------------------*/
volatile bool fFirstStartup = TRUE;
volatile bool fInitDone = FALSE;
volatile bool fIsStandAlone = 0;
volatile delay_t TimingDelay = 0;

/* Private function prototypes -----------------------------------------------*/
void TIM_Configuration( void );
void Specific_Init( void );
void asmCall( void );

/* External functions --------------------------------------------------------*/


/******************************************************************************
*
*                                main
*
******************************************************************************/
/**
*  CircleOS main function.
*  Initialize hardware and wait for interrupts for ever.
**/
/******************************************************************************/
int main( void )
{
    fInitDone = 0;  /* Flag to tell that initialisation is still active (0) or done (1) */

    /* To detect a first power up (battery has been just switched off, */
    /* we look whether the first x backup registers contain only 00 */
    fFirstStartup = !UTIL_isBackupRegisterConfigured();

    // Specific hardware initialization
    Specific_Init();

    /* Launch SysTick to 1 ms => scheduler  */
    SysTick_Configuration();

#if MEMS_POINTER     // Not necessary if no MEMS menu handling
    /* Initialize the SPI MEMS driver   */
    MEMS_Init();
#endif

//fIsStandAlone = 1;

    // The fIsStandAlone flag is set during MEMS initialization
    // We jump over other initialization in order to minimize time in high power mode
    if ( !fIsStandAlone )
    {
        /* LED configuration */
        LED_Init();

        /* RTC configuration */
        RTC_Init();

        /* Button configuration */
        BUTTON_Init();

        /* LCD configuration */
        LCD_Init();

        /* Audio codec init */
#if AUDIO_AVAIL
        AUDIO_Init();
#endif

        /* Power manager init */
#if POWER_MNGT
        POWER_Init();
#endif

        /* BUZZER configuration  */
        /* Note : for Primers with audio buzzer, */
        /* BUZZER_Init must be done AFTER AUDIO_Init */
        BUZZER_Init();

        /* ADC configuration */
        ADConverter_Init();

        /* DRAW configuration */
        DRAW_Init();

#if TOUCHSCREEN_AVAIL
        /* Touchscreen init */
        TOUCHSCR_Init();

        TOOLBAR_Init();
        /* Note TOOLBAR_Init must be done AFTER AUDIO_Init, BUZZER_Init and TOUCHSCR_Init */
#endif // Touchscreen

        /* Menu init */
        MENU_Init();

        /* Configure TIMs */
#if MEMS_SPEC_MNGT
        TIM_Configuration();
        /* Note TIM_Configuration must be done AFTER MEMS_Init */
#endif        
    }

    // Launch the previous appli if configured
    AutorunOn = (( UTIL_ReadBackupRegister( BKP_SYS2 ) ) & BKPMASK_S2_AUTORUN ) ? 1 : 0;
    if ( AutorunOn || fIsStandAlone )
    {
        // Retrieve the last appli index
        CurrentApplication = UTIL_ReadBackupRegister( BKP_SYS1 );

        // Try to launch the last appli
        APP_LaunchAppli( CurrentApplication );
    }

    fInitDone = 1;


#if AUDIO_AVAIL
    if ( !AutorunOn )
    {
        UTIL_SetPll( UTIL_GetPll() );
        AUDIO_Welcome_Msg();
    }
#endif //AUDIO_AVAIL 

    /* Loop for ever, waiting for interrupts. */
                    BUZZER_SetMode(BUZZER_ON);
                    BUZZER_SetMode(BUZZER_PLAYMUSIC);
                    BUZZER_PlayMusic(Melody1);

    while ( 1 )
    {                
                    
                    x = MEMS_GetInfo()->OutX_F4 >> 2;
                    x=x*90/1000;

                    if (x<0) { x=-x; }     
                
                    if (x < 18) {
                        aagg = 0;
                    } else if(x>=18 && x<36) {
                        aagg = 1;
                    } else if(x>=36 && x<54) {
                        aagg = 2;
                    } else if(x>=54 && x<72) {
                        aagg = 3;
                    } else {
                        aagg = 4;
                    }
        //Sleep_Call(); /*To save energy while doing nothing...   */
    }

    return 0;
}

