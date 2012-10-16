/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     menu_app_spe.c
* @brief    General functions to handle the 'Application menu'.
* @author   FL
* @date     07/2007
* @version  4.0
* @date     05/2010 Folder reorganization
*
* @note     Platform = Primer1 & 2
*
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"
#include "menu_app_spe.h"

/// @cond Internal

/* Private functions ---------------------------------------------------------*/

#ifdef PRIMER1
/*******************************************************************************
*
*                                APP_PrevApp
*
*******************************************************************************/
/**
*  Sets the CurrentApplication variable to the previous one in the list
*
*  @return MENU_code return value from FctApp()
*
**/
/******************************************************************************/
NODEBUG static enum MENU_code APP_PrevApp( void )
{
    if ( CurrentApplication == LastApplication )
    {
        CurrentApplication = 0;
    }
    else
    {
        CurrentApplication --;
    }

    if ((( *ApplicationTable )[ -CurrentApplication ] ) == APP_VOID )
    {
        CurrentApplication = 0 ;
    }

    UTIL_WriteBackupRegister( BKP_SYS1, CurrentApplication );

    return FctApp();
}

/*******************************************************************************
*
*                                APP_NextApp
*
*******************************************************************************/
/**
*  Sets the CurrentApplication variable to the next one in the list
*
*  @return  MENU_code from FctApp()
*
**/
/******************************************************************************/
NODEBUG static enum MENU_code APP_NextApp( void )
{
    CurrentApplication ++;

    if ((( *ApplicationTable )[ -CurrentApplication ] ) == APP_VOID )
    {
        CurrentApplication = 0 ;
    }

    UTIL_WriteBackupRegister( BKP_SYS1, CurrentApplication );

    return FctApp();
}

/* Public functions for CircleOS ---------------------------------------------*/

/*******************************************************************************
*
*                                FctApp
*
*******************************************************************************/
/**
*   ?
*
*  @retval  MENU_CHANGE
*
**/
/******************************************************************************/
NODEBUG enum MENU_code FctApp( void )
{

    s32         i;
    tMenuItem*  curapp;

    // Points to the 65th vector in FLASH (see CircleStartup.c)
    //note that these pointers are not used as irq vector since
    //the active table is in RAM
    unsigned long* lptr = ( unsigned long* ) CIRCLEOS_FAT_ADDRESS;

    LastApplication =  -1;  //by default no application

    ApplicationTable  = ( tMenuItem * ((* )[] ) )( *lptr );

    for ( i = 0; i < MAXAPP; i++ )
    {
        if (( *ApplicationTable )[ -i ] == APP_VOID ) //has been programmed
        {
            break;
        }

        LastApplication =  i;
    }


    // Check current application.
    CurrentApplication = UTIL_ReadBackupRegister( BKP_SYS1 );

    if (( LastApplication ==  -1 ) || ( CurrentApplication > LastApplication ) )
    {
        CurrentApplication = 0;
    }

    if (( *ApplicationTable )[ -CurrentApplication ] != APP_VOID )
    {
        addr   = ( long unsigned )( *ApplicationTable )[ -CurrentApplication ] ;
        addr &= 0x00FFFFFF;
        addr |= 0x08000000;
        curapp = ( tMenuItem* ) addr;

        if ( LastApplication > ( MAX_MENUAPP_SIZE - 1 ) )
        {
            AppMenu.NbItems = 4;

            AppMenu.Items[0].Text         = "Previous";
            AppMenu.Items[0].Fct_Init     = APP_PrevApp;

            AppMenu.Items[1]              = *curapp;

            AppMenu.Items[2].Text         = "Next";
            AppMenu.Items[2].Fct_Init     = APP_NextApp;

            AppMenu.Items[3].Text         = "Quit";
            AppMenu.Items[3].Fct_Init     = MENU_Quit;
        }
        else
        {
            AppMenu.NbItems = LastApplication + 2;
            for ( i = 0; i <= LastApplication; i++ )
            {
                addr   = ( long unsigned )( * ApplicationTable )[ -i ] ;
                addr &= 0x00FFFFFF;
                addr |= 0x08000000;

                curapp = ( tMenuItem* ) addr;
                AppMenu.Items[i]         = *curapp;
            }

            AppMenu.Items[i].Text      = "Quit";
            AppMenu.Items[i].Fct_Init  = MENU_Quit;
        }
    }
    else
    {
        AppMenu.NbItems = 2;

        AppMenu.Items[0].Text      = "No applic.";
        AppMenu.Items[0].Fct_Init  = MENU_Quit;
        AppMenu.Items[1].Text      = " loaded!  ";
        AppMenu.Items[1].Fct_Init  = MENU_Quit;
    }

    MENU_Set( &AppMenu );

    return MENU_CHANGE;
}
#endif

#ifdef PRIMER2
/*******************************************************************************
*
*                                FctAppIni
*
*******************************************************************************/
/**
*  Show the list of loaded applications
*
*  @retval  MENU_CONTINUE_COMMAND
*
**/
/******************************************************************************/
NODEBUG enum MENU_code FctAppIni( void )
{
    s32         i;
    tMenuItem*  curapp;

    NoAppLoaded = FALSE;

    // Points to the 65th vector in FLASH (see CircleStartup.c)
    //note that these pointers are not used as irq vector since
    //the active table is in RAM
    unsigned long* lptr = ( unsigned long* ) CIRCLEOS_FAT_ADDRESS;

    LastApplication =  -1;  //by default no application
    ApplicationTable  = ( tMenuItem * ((* )[] ) )( *lptr );

    for ( i = 0; i < MAXAPP; i++ )
    {
        if (( *ApplicationTable )[ -i ] == APP_VOID ) //has been programmed
        {
            LastApplication =  i - 1;
            break;
        }
    }

    // Check that current application is still in valid range.
    CurrentApplication = UTIL_ReadBackupRegister( BKP_SYS1 );
    if (( LastApplication ==  -1 ) || ( CurrentApplication > LastApplication ) )
    {
        CurrentApplication = 0;
    }

#if 0 // 1 For testing long list management
    AppMenu.NbItems = 60;
#else
    if (( *ApplicationTable )[ -CurrentApplication ] != APP_VOID )
    {
        addr   = ( long unsigned )( *ApplicationTable )[ -CurrentApplication ] ;
        addr &= 0x00FFFFFF;
        addr |= 0x08000000;
        curapp = ( tMenuItem* ) addr;

        AppMenu.NbItems = LastApplication + 2;
        for ( i = 0; i <= LastApplication; i++ )
        {
            addr  = ( long unsigned )( * ApplicationTable )[ -i ] ;
            addr &= 0x00FFFFFF;
            addr |= 0x08000000;

            curapp = ( tMenuItem* ) addr;
            AppMenu.Items[i].Text  = ( *curapp ).Text;
        }

        AppMenu.Items[i].Text      = "Quit";
    }
    else
    {
        AppMenu.NbItems = 2;
        AppMenu.Items[0].Text      = "No applic.";
        AppMenu.Items[1].Text      = " loaded!  ";
        NoAppLoaded = TRUE;
    }
#endif
    // Accelerate the menu divider
    MENU_SetAppliDivider( 10 );

    // Show the applications in centered list form
    LIST_Set( &AppMenu, 0, 0, 1 );

    return MENU_CONTINUE_COMMAND;
}


/*******************************************************************************
*
*                                FctAppMgr
*
*******************************************************************************/
/**
*   Manage the selection of the application to be launch
*
*  @retval  MENU_CONTINUE
*
**/
/******************************************************************************/
NODEBUG enum MENU_code FctAppMgr( void )
{
    index_t appSel;

    // Manage the touchcreen list
    appSel = LIST_Manager();

    // If choice done
    if ( appSel != -1 )
    {
#if 0               // For testing long list
        return fQuit();     // fl test
#endif

        // Detach scroll list
        CurrentList = 0;

        // If choice different than quit
        if (( appSel != ( AppMenu.NbItems - 1 ) ) && !NoAppLoaded )
        {
            // Save the index of the current appli
            UTIL_WriteBackupRegister( BKP_SYS1, appSel );

            return APP_LaunchAppli( appSel );
        }
        else
        {
            // Restore the menu divider
            MENU_RestoreAppliDivider();
            return fQuit();
        }
    }
    return MENU_CONTINUE;
}


#endif


/*******************************************************************************
*
*                                LaunchAppli
*
*******************************************************************************/
/**
*   Launch an application
*
*  @param[in] index = index of the application in the list
*
**/
/******************************************************************************/
NODEBUG enum MENU_code APP_LaunchAppli( index_t index )
{
    if ( index > MAXAPP )
    {
        index = 0;
    }

    // Search the adress of the selected application
    unsigned long* lptr = ( unsigned long* ) CIRCLEOS_FAT_ADDRESS;
    ApplicationTable  = ( tMenuItem * ((* )[] ) )( *lptr );

    // First, we search for extra applications;
    if (( *ApplicationTable )[ -index ] != APP_VOID )
    {
        addr   = ( long unsigned )( *ApplicationTable )[ -index ] ;
        addr &= 0x00FFFFFF;
        addr |= 0x08000000;
        CurrentCommand = ( tMenuItem* ) addr;

        // Check validity of the application
        if (( CurrentCommand == APP_VOID ) || ( CurrentCommand == 0 ) )
            return fQuit();

        // Disable menu and screen rotation
        fDisplayTime = 0;
        CurrentMenu = 0;
        BUTTON_SetMode( BUTTON_ONOFF );
        POINTER_SetMode( POINTER_OFF );
        LCD_SetRotateScreen( 0 );
        DRAW_Clear();

#ifdef PRIMER2
        // Disable scribble touchscreen mode
        TOUCHSCR_SetMode( TS_NORMAL );
#endif

        // Restore the menu divider
        MENU_RestoreAppliDivider();

        // Launch the selected application
        return CurrentCommand->Fct_Init();
    }

    return fQuit();
}

/*******************************************************************************
*
*                                APP_FindAppIndex
*
*******************************************************************************/
/**
*   Read the FAT in order to find the aplication corresponding to the menu index
*
*  @param[in] index = index of the application in the menu
*  @return    index of the application in the table
*  @note      for compatibility with other platforms
*
**/
/******************************************************************************/
NODEBUG index_t APP_FindAppIndex( index_t index )
{
    return index;
}


/// @endcond
