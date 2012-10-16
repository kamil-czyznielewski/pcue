/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     menu_app.h
* @brief    Header for menu_app.c
* @author   FL
* @date     07/2007
*
**/
/******************************************************************************/


/* Private defines -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
s32            CurrentApplication      = 0;
tMenuItem*(( *ApplicationTable )[] );
index_t        LastApplication;
long unsigned  addr;
bool           NoAppLoaded;

#ifdef PRIMER1
tMenu AppMenu =
{
    1,
    "Applications",
    5, 0, 0, 0, 0, 0,
    0
} ;
#endif

#ifdef PRIMER2   // For testing long list with touchscreen
tList AppMenu =
{
    1,
    "Applications",
    40,
    0, 0, 0, 0, 0,
    6,
    0,
    0,
#if 0 //only for debugging lists        
    "Appli  0",
    "Appli  1",
    "Appli  2",
    "Appli  3",
    "Appli  4",
    "Appli  5",
    "Appli  6",
    "Appli  7",
    "Appli  8",
    "Appli  9",
    "Appli 10",
    "Appli 11",
    "Appli 12",
    "Appli 13",
    "Appli 14",
    "Appli 15",
    "Appli 16",
    "Appli 17",
    "Appli 18",
    "Appli 19",
    "Appli 20",
    "Appli 21",
    "Appli 22",
    "Appli 23",
    "Appli 24",
    "Appli 25",
    "Appli 26",
    "Appli 27",
    "Appli 28",
    "Appli 29",
    "Appli 30",
    "Appli 31",
    "Appli 32",
    "Appli 33",
    "Appli 34",
    "Appli 35",
    "Appli 36",
    "Appli 37",
    "Appli 38",
    "Appli 39",
    "Appli 40",
    "Appli 41",
    "Appli 42",
    "Appli 43",
    "Appli 44",
    "Appli 45",
    "Appli 46",
    "Appli 47",
    "Appli 48",
    "Appli 49",
    "Appli 50",
    "Appli 51",
    "Appli 52",
    "Appli 53",
    "Appli 54",
    "Appli 55",
    "Appli 56",
    "Appli 57",
    "Appli 58",
    "Appli 59",
    "Appli 60",
    "Appli 61",
    "Appli 62",
    "Appli 63",
#endif
} ;
#endif


/* Public function prototypes ------------------------------------------------*/
extern enum MENU_code FctApp( void );
extern enum MENU_code FctAppIni( void );
extern enum MENU_code FctAppMgr( void );
extern enum MENU_code LaunchAppli( index_t index );

/* Private function prototypes -----------------------------------------------*/
