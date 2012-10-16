/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     menu_app_spe.c
* @brief    General functions to handle the 'Application menu'.
* @author   YRT
* @date     09/2009
* @note     Platform = Open4 STM32
*
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"
#include "menu_app_spe.h"

/// @cond Internal

/* Private functions ---------------------------------------------------------*/

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
enum MENU_code FctAppIni( void )
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

#if LIST_TEST // 1 For testing long list management
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
enum MENU_code FctAppMgr( void )
{
    index_t appSel;

    // Manage the touchcreen list
    appSel = LIST_Manager();

    // If choice done
    if ( appSel != -1 )
    {
#if LIST_TEST       // For testing long list
        return fQuit();
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
enum MENU_code APP_LaunchAppli( index_t index )
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
        DRAW_Clear();
        LCD_SetRotateScreen( 0 );

        // Disable scribble touchscreen mode
        TOUCHSCR_SetMode( TS_NORMAL );

        // Restore the menu divider
        MENU_RestoreAppliDivider();

        // By default, set the offset for application compatibility with Open4
        if ( UTIL_GetPrimerType() > 2 )
            LCD_SetOffset( OFFSET_ON );

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
index_t APP_FindAppIndex( index_t index )
{
    return index;
}


/// @endcond
