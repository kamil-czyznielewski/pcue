/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file       fs.c
* @author     S. S. Brandsma
* @date       23-01-2009
* @brief      Filesystem OS layer
* @version    1.0
*
* @author     YRT
* @date       05/02/2009
* @version    1.1
**/
/*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

#if SDCARD_SDIO
#include "sdcard.h"
#endif
#include "fs.h"
#include <string.h>

/// @cond Internal

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define MAX_DIRS 30
#define MAX_PATH_LENGTH 200

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
//static u8  sector[SECTOR_SIZE];
static union array
{
    u8 sector[SECTOR_SIZE];    // the char array
    unsigned int  i;           // force 32-bit int alignment
} u;
static u32 StartMBR;
static u8  SDCardfault = 0;

/* Public variables ----------------------------------------------------------*/
VOLINFO volume_info;
DIRINFO directory_info;
DIRENT directory_entry;
u8* FS_PathFilter = 0;

struct DirItem
{
    u8 Name[14];
    u8 flags;
};

struct DirItem DirItems[MAX_DIRS];
tList DirMenu =    {    1,    "SDCard",    30,    0, 0, 0, 0, 0,    6,    0,    0,    };

u8 level = 0;
u8 CurrentPath[MAX_PATH_LENGTH];


/* Private functions ----------------------------------------------------------*/
void NVIC_Config_SDIO( void );

/*******************************************************************************
*
*                            FS_Explorer_UpdateList
*
*******************************************************************************/
/**
*
*                 Populate the list with contents of current directory.
*
*   Note: the global variable FS_filter contents a filter on the file type
*
*******************************************************************************/
NODEBUG2 void FS_Explorer_UpdateList( void )
{
    u8 i = 0;
    u32 res;

    DFS_memset(( void* ) DirItems, 0, sizeof( DirItems ) );

    /* Loop through root dir and display files / dirs*/
    while ( !( res = FS_GetNextEntry( &volume_info, &directory_info, &directory_entry ) ) )
    {
        if ( directory_entry.name[0] )
        {
            /* Directory*/
            if ( directory_entry.attr & FS_ATTR_DIRECTORY )
            {
                if ( strncmp( directory_entry.name, ". ", 2 ) ) /* don't display current "." directory*/
                {
                    strcat( DirItems[i].Name, "[" );
                    strncat( DirItems[i].Name, directory_entry.name, 11 );
                    strcat( DirItems[i].Name, "]" );

                    DirMenu.Items[i].Text = DirItems[i].Name;
                    DirItems[i].flags |= FS_ATTR_DIRECTORY; /* set flag in local list*/
                    i++;
                }
            }
            else /* File*/
            {
                /* Check if matches the type filter*/
                if (( FS_PathFilter == 0 ) || ( strncmp( directory_entry.name + 8, FS_PathFilter, 3 ) == 0 ) )
                {
                    DFS_strncpy( DirItems[i].Name, directory_entry.name, 11 );
                    DirMenu.Items[i].Text = DirItems[i].Name;
                    i++;
                }
            }

            if ( i >= MAX_DIRS )
                break;
        }


    }

    if ( i == 0 ) /* empty dir ?*/
    {
        DFS_memset(( void* )DirItems[0].Name, 0, 11 );
        DFS_strcpy( DirItems[i].Name, "[..]" );
        DirMenu.Items[i].Text = DirItems[i].Name;
        DirItems[i].flags |= FS_ATTR_DIRECTORY; /* set flag in local list*/
    }

    /* Set number of items in list (i + cde "Quit")*/
    DirMenu.Items[i].Text = "Quit";
    i++;
    DirMenu.NbItems = i;
    DirMenu.NbDisp = i > 7 ? 7 : i;

    DirMenu.XPos  = 0;
    DirMenu.YPos  = 150;
    DirMenu.XSize = 128;
    DirMenu.YSize = 140;
}


/// @endcond

/* Public functions ----------------------------------------------------------*/

/*******************************************************************************
*
*                            FS_Mount
*
*******************************************************************************/
/**
*
*  Initializes and connects selected device to file system.
*
*  @param[in]     device (currently only supported: MMCSD_SDIO).
*
*  @return        MENU_CONTINUE_COMMAND.
*
**/
/*******************************************************************************/
u32 FS_Mount( enum STORAGE_device device )
{
#if SDCARD_SDIO
    NVIC_Config_SDIO();
#endif
    return DFS_Mount( device );
}


/*******************************************************************************
*
*                            FS_Unmount
*
*******************************************************************************/
/**
*   Deinitializes and disconnects selected device to file system.
*
*   @param[in]       device (currently only supported: MMCSD_SDIO).
*   @return          MENU_CONTINUE_COMMAND.
*
*   @warning         Not yet implemented.
*
**/
/*******************************************************************************/
u32 FS_Unmount( enum STORAGE_device device )
{
    /* not yet implemented*/
}


/*******************************************************************************
*
*                            FS_OpenFile
*
*******************************************************************************/
/**
*                   Open file.
*
*                   Call FS_OpenFile with mode = FS_READ and supply a path and the relevant
*                   VOLINFO structure. FS_OpenFile will populate a FILEINFO that can be used
*                   to refer to the file.
*                   Optionally call FS_Seek to set the file pointer. If you attempt to set
*                   the file pointer past the end of file, the file will NOT be extended. Check
*                   the FILEINFO.pointer value after FS_Seek to verify that the pointer is
*                   where you expect it to be.
*                   Observe that functionality similar to the "whence" parameter of fseek() can
*                   be obtained by using simple arithmetic on the FILEINFO.pointer and
*                   FILEINFO.filelen members.
*                   Call FS_ReadFile with the FILEINFO you obtained from OpenFile, and a
*                   pointer to a buffer plus the desired number of bytes to read, and a
*                   pointer to a sector-sized scratch buffer. The reason a scratch sector is
*                   required is because the underlying sector read function doesn't know
*                   about partial reads.
*                   Note that a file opened for reading cannot be written. If you need r/w
*                   access, open with mode = FS_WRITE (see below).
*
* @param[in]        volinfo  : structure populated with information about filesystem.
* @param[in]        *path    : string containing the path of the file to access.
* @param[in]        mode     : access mode = FS_READ or FS_WRITE.
* @param[out]       fileinfo : empty structure filled by the function, for further access.
*
* @return           MENU_CONTINUE_COMMAND.
**/
/*******************************************************************************/
u32 FS_OpenFile( PVOLINFO volinfo, u8* path, u8 mode, PFILEINFO fileinfo )
{
    return DFS_OpenFile( volinfo, path, mode, u.sector, fileinfo );
}



/*******************************************************************************
*
*                            FS_ReadFile
*
*******************************************************************************/
/**
*                   Read file.
*
* @param[in]        fileinfo      : structure populated by the DFS_OpenFile function.
* @param[out]       *buffer       : buffer filled with the read result.
* @param[out]       *successcount : number of bytes really read.
* @param[in]        len           : number of bytes to read.
*
* @return           MENU_CONTINUE_COMMAND.
*
**/
/*******************************************************************************/
u32 FS_ReadFile( PFILEINFO fileinfo,  u8* buffer, u32* successcount, u32 len )
{
    return DFS_ReadFile( fileinfo, u.sector, buffer, successcount, len );
}


/*******************************************************************************
*
*                            FS_WriteFile
*
*******************************************************************************/
/**
*                   Write file.
*
*                   Call DFS_OpenFile with mode = DFS_WRITE and supply a path and the relevant
*                   VOLINFO structure. DFS_OpenFile will populate a FILEINFO that can be used to
*                   refer to the file.
*                   Optionally call DFS_Seek to set the file pointer. Refer to the notes on
*                   this topic in the section on reading files, above.
*                   Call DFS_WriteFile with the FILEINFO you obtained from OpenFile, and a
*                   pointer to the source buffer, and a pointer to a sector-sized scratch
*                   buffer.
*                   Note that a file open for writing can also be read.
*                   Files are created automatically if they do not exist. Subdirectories are
*                   NOT automatically created.
*                   If you open an existing file for writing, the file pointer will start at
*                   the beginning of the data; if you want to append, seek to the end before
*                   writing new data.
*                   If you perform random-access writes to a file, the length will NOT change
*                   unless you exceed the file's original length. There is currently no
*                   function to truncate a file at the current pointer position.
*                   On-disk consistency is guaranteed when DFS_WriteFile exits, unless your
*                   physical layer has a writeback cache in it.
*
* @param[in]        fileinfo      : structure populated by the DFS_OpenFile function.
* @param[in]        buffer        : buffer to be written.
* @param[out]       *successcount : number of bytes really read.
* @param[in]        len           : number of bytes to read.
*
* @return           MENU_CONTINUE_COMMAND.
*
**/
/*******************************************************************************/
u32 FS_WriteFile( PFILEINFO fileinfo,  u8* buffer, u32* successcount, u32 len )
{
    return DFS_WriteFile( fileinfo, u.sector, buffer, successcount, len );
}


/*******************************************************************************
*
*                            FS_Close
*
*******************************************************************************/
/**
*                    Close file.
*
*   @param[in]       fileinfo      : structure populated by the DFS_OpenFile function.
*   @return          MENU_CONTINUE_COMMAND.
*
*   @warning         Not yet implemented.
*
**/
/*******************************************************************************/
u32 FS_Close( PFILEINFO fileinfo )
{
    /* not implemented ;*/
}


/*******************************************************************************
*
*                            FS_Seek
*
*******************************************************************************/
/**
*                    Seek file pointer to a given position.
*
*   @param[in]       fileinfo  : structure populated by the DFS_OpenFile function.
*   @param[in]       offset    : position to reach.
*   @return          This function does not return status.
*                    Refer to the fileinfo->pointer value.
*
**/
/*******************************************************************************/
void FS_Seek( PFILEINFO fileinfo, u32 offset )
{
    DFS_Seek( fileinfo, offset, u.sector );
}


/*******************************************************************************
*
*                            FS_Delete
*
*******************************************************************************/
/**
*                    Delete file or directory.
*
*                    FS_Delete will delete a subdirectory (correctly) but will NOT
*                    first recurse the directory to delete the contents.
*                    So you will end up with lost clusters..
*
*   @param[in]       volinfo  : structure populated with information about filesystem.
*   @param[in]       *path    : string containing the path of the file to delete.
*
*   @return          MENU_CONTINUE_COMMAND.
*
**/
/*******************************************************************************/
u32 FS_Delete( PVOLINFO volinfo, u8* path )
{
    return DFS_UnlinkFile( volinfo, path, u.sector );
}


/*******************************************************************************
*
*                            FS_GetNextEntry
*
*******************************************************************************/
/**
*                   Search next entry of the directory.
*
*                   Call FS_GetNextEntry to receive the DIRENT contents for the next directory
*                   item. This function returns FS_OK for no error, and FS_EOF if there
*                   are no more entries in the directory being searched.
*                   Before using the DIRENT, check the first character of the name. If it
*                   is NULL, then this is an unusable entry - call DFS_GetNext again to
*                   keep searching. LFN directory entries are automatically tagged this way
*                   so your application will not be pestered by them.
*
*   @note           A designed side-effect of this code is that when you locate the
*                   file of interest, the DIRINFO.currentcluster, DIRINFO.currentsector
*                   and DIRINFO.currententry-1 fields will identify the directory entry of.
*
*   @param[in]       volinfo  : structure populated with information about filesystem.
*   @param[in]       dirinfo  : .
*   @param[out]      dirent   : next directory entry.
*
*   @return          MENU_CONTINUE_COMMAND.
*
**/
/*******************************************************************************/
u32 FS_GetNextEntry( PVOLINFO volinfo, PDIRINFO dirinfo, PDIRENT dirent )
{
    return DFS_GetNext( volinfo, dirinfo, dirent );
}


/*******************************************************************************
*
*                            FS_OpenDirectory
*
*******************************************************************************/
/**
*                  Open directory.
*
*                  Call FS_Opendir and supply a path, populated VOLINFO and a
*                  DIRINFO structure to receive the results.
*                  This buffer must remain unmolested while you have the directory open
*                  for searching.
*
*   @param[in]     volinfo  : structure populated with information about filesystem.
*   @param[in]     *dirname : string containing the name of the directory.
*   @param[out]    dirinfo  : structure with infos about the directory.
*
*   @return        MENU_CONTINUE_COMMAND.
*
**/
/*******************************************************************************/
u32 FS_OpenDirectory( PVOLINFO volinfo, u8* dirname, PDIRINFO dirinfo )
{
    dirinfo->scratch = u.sector;  /* DFS Note - you must PREPOPULATE*/
    /* the DIRINFO.scratch field with a pointer to a sector scratch buffer.*/
    return DFS_OpenDir( volinfo, dirname, dirinfo );
}


/*******************************************************************************
*
*                            FS_GetVolumeInfo
*
*******************************************************************************/
/**
*                  Gets the volume informations.
*
*                  Call FS_GetVolInfo to read filesystem info into a VOLINFO structure.
*                  FS_GetVolInfo needs to know the unit number and partition starting
*                  sector (as returned by DFS_GetPtnStart, or 0 if this is a "floppy-
*                  format" volume without a MBR).
*
*   @param[in]     unit        : .
*   @param[in]     startsector : .
*   @param[out]    volinfo     : structure populated with information about filesystem.
*
*   @return        MENU_CONTINUE_COMMAND.
*
**/
/*******************************************************************************/
u32 FS_GetVolumeInfo( u8 unit, u32 startsector, PVOLINFO volinfo )
{
    return DFS_GetVolInfo( unit, u.sector, startsector, volinfo );
}

/*******************************************************************************
*
*                            FS_Explorer_Ini
*
*******************************************************************************/
/**
*                Checks the presence of the SDCARD,
*                initializes various structures
*                and lists the root directory.
*
*   @return      MENU_CONTINUE_COMMAND
*
*******************************************************************************/
enum MENU_code FS_Explorer_Ini( void )
{

    DFS_memset(( void* ) CurrentPath, 0, sizeof( CurrentPath ) );

    SDCardfault = 0;

    /* Mount MMCSD */
    StartMBR = FS_Mount( MMCSD_SDIO );

    if ( StartMBR == 0xFFFFFFFF )
    {
        DirMenu.Items[0].Text = "No SDCARD";
        SDCardfault = 1;
    }

    if ( !SDCardfault )
    {
        /* Open volume on first partition (0)*/
        if ( FS_GetVolumeInfo( 0, StartMBR, &volume_info ) )
        {
            DirMenu.Items[0].Text = "Err: GetVolInfo";
            SDCardfault = 1;
        }
    }

    if ( !SDCardfault )
    {
        /* Open root directory*/
        if ( FS_OpenDirectory( &volume_info, "", &directory_info ) )
        {
            DirMenu.Items[0].Text = "Err: Open Root";
            SDCardfault = 1;
        }
    }

    if ( SDCardfault )
    {
        /* 2 items : error msg + quit*/
        DirMenu.NbItems = 2;
        DirMenu.Items[1].Text = "Quit";

        /* Show list*/
        LIST_Set( &DirMenu, 0, 0, 1 );
    }
    else
    {

        /* Populate the list*/
        FS_Explorer_UpdateList();

        MENU_SetAppliDivider( 10 );

        /* Show list*/
        LIST_Set( &DirMenu, 0, 0, 1 );

    }

    return MENU_CONTINUE_COMMAND;

}


/*******************************************************************************
*
*                            FS_Explorer
*
*******************************************************************************/
/**
*                 Navigation into the SDCARD folders, through a list,
*                 and selection of a file.
*
*    @return      -1 if navigation not finished,
*                 0 if Quit request,
*                 1 if file selected : use FS_GetSDCardCurrentPath() to get the path.
*
*    @note        This function has to be called periodically until it
*                 returns a value equal to -1.
*
*    @warning     FS_Explorer_Ini() must be called before.
*
*******************************************************************************/
s32 FS_Explorer( void )
{
    s32 i;
    u32 retval;
    u8 EOP = 0;

    /* Call the List manager of the current list*/
    i = LIST_Manager();

    /* If item selected*/
    if ( i != -1 )
    {

        /* If choice different than quit and card OK*/
        if (( i != ( DirMenu.NbItems - 1 ) ) && !SDCardfault )
        {
            u8 dir[MAX_PATH_LENGTH];
            DFS_memset( dir, 0, sizeof( dir ) );

            /* Directory selected from list*/
            if ( DirItems[i].flags & FS_ATTR_DIRECTORY )
            {
                /* Check if we want to go up a level*/
                if ( !( strncmp( DirItems[i].Name + 1, "..", 2 ) ) )
                {
                    u8* ptr = CurrentPath + ( sizeof( CurrentPath ) - 1 );


                    /* Remove directory name from CurrentPath*/
                    for ( i = MAX_PATH_LENGTH; i > 0; i-- )
                    {
                        if (( *ptr == '/' ) && ( !EOP ) )
                        {
                            /* Found end of current path    */
                            EOP = 1;
                        }
                        else if (( *ptr == '/' ) && ( EOP ) )
                        {
                            break; /* erased subdir and found //*/
                        }
                        *ptr-- = 0;
                    }
                    DFS_strcpy( dir, CurrentPath );
                }
                else
                {
                    DFS_strcpy( dir, CurrentPath );
                    strncat( dir, DirItems[i].Name + 1, 11 ); /* strip '[ ]'*/
                }

                /* Open directory*/
                if ( retval = FS_OpenDirectory( &volume_info, dir, &directory_info ) )
                {
                    /* Cannot open dir*/
                    /* add error handling. Go back to root for now*/
                    if ( retval == 3 )
                    {
                        FS_OpenDirectory( &volume_info, "", &directory_info );
                    }
                    /* Reset CurrentPath                */
                    DFS_memset(( void* ) CurrentPath, 0, sizeof( CurrentPath ) );
                }
                else
                {
                    /* Check if we didn't go up*/
                    if ( !EOP )
                    {
                        u8 len = my_strlen( CurrentPath );
                        u8 j;


                        /* Remove trailing spaces*/
                        for ( j = len; j > 0; j-- )
                        {
                            if ( CurrentPath[j] == 0x20 )
                                CurrentPath[j] = 0;
                        }

                        /* Add new dir to current path*/
                        strncat( CurrentPath, DirItems[i].Name + 1, 11 ); /* strip '[ ]'*/

                        len = my_strlen( CurrentPath );
                        /* Remove trailing spaces again*/
                        for ( j = len; j > 0; j-- )
                        {
                            if ( CurrentPath[j] == 0x20 )
                                CurrentPath[j] = 0;
                        }

                        /* Add dir seperator*/
                        strcat( CurrentPath, "/" );
                    }
                }

                /* Update directory list*/
                FS_Explorer_UpdateList();

                /* Clear display*/
                DRAW_Clear();

                /* Show list*/
                LIST_Set( &DirMenu, 0, 0, 1 );

            } /* end (DirItems[i].flags & FS_ATTR_DIRECTORY)*/
            else
                /* File selected from list*/
            {
                /* Add file name to the path*/
                strncat( CurrentPath, DirItems[i].Name, 11 );
                return 1;
            }

        } /* end (i != (DirMenu.NbItems - 1) )  && !SDCardfault)*/
        else
        {
            /* Quit requested*/
            return 0;
        }

    } /* end (i != -1)*/

    return -1;
}

/*******************************************************************************
*
*                            FS_GetSDCardCurrentPath
*
*******************************************************************************/
/**
*                 Get the Currentpath of the SDCard,
*                 updated during navigation with the explorer.
*
*    @return      currentpath.
*
*    @warning     FS_Explorer_Ini() must have been called before.
*
*******************************************************************************/
u8* FS_GetSDCardCurrentPath( void )
{
    return ( u8* ) &CurrentPath;
}


/*******************************************************************************
*
*                            FS_GetSDCardVolInfo
*
*******************************************************************************/
/**
*                 Get the volume informations of the SDCard,
*                 structure populated by the FS_Explorer_Ini().
*                 This structure is necessary for file access,
*                 (FS_OpenFile, FS_ReadFile, FS_Seek...).
*
*    @return      volume_info.
*
*    @warning     FS_Explorer_Ini() must have been called before.
*
*******************************************************************************/
VOLINFO* FS_GetSDCardVolInfo( void )
{
    return &volume_info;
}

/*******************************************************************************
*
*                            FS_GetPathFilter
*
*******************************************************************************/
/**
*                 Get the filter applied to the file type during exploring
*                 the SDCard
*
*    @return      FS_PathFilter.
*
*******************************************************************************/
u8* FS_GetPathFilter( void )
{
    return FS_PathFilter;
}

/*******************************************************************************
*
*                            FS_SetPathFilter
*
*******************************************************************************/
/**
*                 Set the filter applied to the file type during exploring
*                 the SDCard.
*                 A null pointer indicates no filter
*
*    @param[in]   filter : pointer on a string containing the filter.
*
*    Note : the string must contain 3 char.
*
*******************************************************************************/
void FS_SetPathFilter( u8* filter )
{
    if ( filter )
        FS_PathFilter = strupr( filter );
    else
        FS_PathFilter = 0;
}

/// @cond Internal
#if SDCARD_SDIO
/*******************************************************************************
* Function Name  : NVIC_Config_SDIO
* Description    : Configures SDIO IRQ channel.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
NODEBUG2 void NVIC_Config_SDIO( void )
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Configure the NVIC Preemption Priority Bits */
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_1 );

    NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init( &NVIC_InitStructure );
}
#endif

/// @endcond