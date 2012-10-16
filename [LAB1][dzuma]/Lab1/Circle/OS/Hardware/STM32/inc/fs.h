/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************
* File Name          :  FS.h
* Author             :  S. S. Brandsma
* Date First Issued  :  23-01-2009
* Description        :  Filesystem OS layer
* Revision           :  1.0
*******************************************************************************/

#include "dosfs.h"

/* Public defines  -----------------------------------------------------------*/

/*===================================================================*/
/* 32-bit error codes*/
#define FS_OK           0           /* no error*/
#define FS_EOF          1           /* end of file (not an error)*/
#define FS_WRITEPROT    2           /* volume is write protected*/
#define FS_NOTFOUND     3           /* path or file not found*/
#define FS_PATHLEN      4           /* path too long*/
#define FS_ALLOCNEW     5           /* must allocate new directory cluster*/
#define FS_ERRMISC      0xffffffff  /* generic error*/


/*===================================================================*/
/* File access modes*/
#define FS_READ         1           /* read-only*/
#define FS_WRITE        2           /* write-only*/

/*===================================================================*/
/* DOS attribute bits*/
#define FS_ATTR_READ_ONLY           0x01
#define FS_ATTR_HIDDEN              0x02
#define FS_ATTR_SYSTEM              0x04
#define FS_ATTR_VOLUME_ID           0x08
#define FS_ATTR_DIRECTORY           0x10
#define FS_ATTR_ARCHIVE         0x20
#define FS_ATTR_LONG_NAME   (FS_ATTR_READ_ONLY | FS_ATTR_HIDDEN | FS_ATTR_SYSTEM | FS_ATTR_VOLUME_ID)

/* Public variables ----------------------------------------------------------*/


/* Public functions ---------------------------------------------------------*/
u32 FS_Mount( enum STORAGE_device device );
u32 FS_Unmount( enum STORAGE_device device );
u32 FS_OpenFile( PVOLINFO volinfo, u8* path, u8 mode, PFILEINFO fileinfo );
u32 FS_ReadFile( PFILEINFO fileinfo,  u8* buffer, u32* successcount, u32 len );
u32 FS_WriteFile( PFILEINFO fileinfo,  u8* buffer, u32* successcount, u32 len );
u32 FS_Close( PFILEINFO fileinfo );
void FS_Seek( PFILEINFO fileinfo, u32 offset );
u32 FS_Delete( PVOLINFO volinfo, u8* path );
u32 FS_GetNextEntry( PVOLINFO volinfo, PDIRINFO dirinfo, PDIRENT dirent );
u32 FS_OpenDirectory( PVOLINFO volinfo, u8* dirname, PDIRINFO dirinfo );
u32 FS_GetVolumeInfo( u8 unit,  u32 startsector, PVOLINFO volinfo );
enum MENU_code FS_Explorer_Ini( void );
s32 FS_Explorer( void );
u8* FS_GetSDCardCurrentPath( void );
VOLINFO* FS_GetSDCardVolInfo( void );
u8* FS_GetPathFilter( void );
void FS_SetPathFilter( u8* filter );
