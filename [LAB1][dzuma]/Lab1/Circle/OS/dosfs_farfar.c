/*
    DOSFS Embedded FAT-Compatible Filesystem
    (C) 2005 Lewin A.R.W. Edwards (sysadm@zws.com)

    You are permitted to modify and/or use this code in your own projects without
    payment of royalty, regardless of the license(s) you choose for those projects.

    You cannot re-copyright or restrict use of the code as released by Lewin Edwards.

    Notes
    =====
    Some platforms may require explicit pragmas or attributes to the structures
    and unions. For example, arm-gcc will require __attribute__ ((__packed__))
    otherwise it will try to be "smart" and place the uint8_t members on 4-byte
    boundaries. There is no truly elegant compiler-independent method to get
    around this sort of problem.

    The code assumes either a von Neumann architecture, or a compiler that
    is smart enough to understand where your pointers are aimed and emit
    the right kind of memory read and write instructions. The implications
    of this statement depend on your target processor and the compiler you
    are using. Be very careful not to straddle bank boundaries on bank-
    switched memory systems.

    Physical 32-bit sector numbers are used throughout. Therefore, the
    CHS geometry (if any) of the storage media is not known to DOSFS. Your
    sector r/w functions may need to query the CHS geometry and perform
    mapping.

    File timestamps set by DOSFS are always 1:01:00am on Jan 1, 2006. If
    your system has a concept of real time, you can enhance this.

    FILEINFO structures contain a pointer to the corresponding VOLINFO
    used to open the file, mainly in order to avoid mixups but also to
    obviate the need for an extra parameter to every file read/write. DOSFS
    assumes that the VOLINFO won't move around. If you need to move or
    destroy VOLINFOs pertaining to open files, you'll have to fix up the
    pointer in the FILEINFO structure yourself.

    The subdirectory delimiter is a forward slash ( '/' ) by default. The
    reason for this is to avoid the common programming error of forgetting
    that backslash is an escape character in C strings; i.e. "\MYDIR\FILE"
    is NOT what you want; "\\MYDIR\\FILE" is what you wanted to type. If you
    are porting DOS code into an embedded environment, feel free to change
    this #define.

    DOSFS does not have a concept of "current directory". A current directory
    is owned by a process, and a process is an operating system concept.
    DOSFS is a filesystem library, not an operating system. Therefore, any
    path you provide to a DOSFS call is assumed to be relative to the root of
    the volume.

    There is no call to close a file or directory that is open for reading or
    writing. You can simply destroy or reuse the data structures allocated for
    that operation; there is no internal state in DOSFS so no cleanup is
    necessary. Similarly, there is no call to close a file that is open for
    writing. (Observe that dosfs.c has no global variables. All state information
    is stored in data structures provided by the caller).

*/


#define NODEBUG __attribute__ ((section(".non_debuggable_code")))

#include "circle_platform.h"

#if SDCARD_SDIO
#include "sdcard.h"             // SDIO interface
#else
#include "msd.h"                // SPI interface
#endif

#include "dosfs.h"

/* Private variables ---------------------------------------------------------*/
#if SDCARD_SDIO
SD_CardInfo SDCardInfo;
SD_Error Status = SD_OK;
#endif
ErrorStatus HSEStartUpStatus;



NODEBUG void* DFS_memset( void* dest, u8 c, s32 count )
{
    u32 i;

    u8* val = ( u8* ) dest;

    for ( i = 0; i < count; i++ )
    {
        *val = c;
        val++;
    }

    return dest;
}


NODEBUG void* DFS_memcpy( void* dest, void* src, s32 count )
{

    u8* d = ( u8* )dest;
    u8* s = ( u8* )src;

    while ( count-- > 0 )
        *d++ = *s++;
    return dest;
}


NODEBUG s32 DFS_memcmp( const void* s1, const void* s2, u32 n )
{
    u8 u1, u2;
    u8* c1 = ( u8* ) s1;
    u8* c2 = ( u8* ) s2;

    for ( ; n-- ; c1++, c2++ )
    {
        u1 = * ( u8* ) c1;
        u2 = * ( u8* ) c2;
        if ( u1 != u2 )
        {
            return ( u1 - u2 );
        }
    }
    return 0;
}

/*NODEBUG s32 DFS_strlen (const u8 *str)
{
    s32 n=0;
    const u8 *s = str;

    while (*s)
    {
        s++;
        n++;
    }
    return(n);
}
*/ /* YRT20080204 => replaced by my_strlen*/

NODEBUG void DFS_strcpy( u8* Dest ,  u8* Src )
{
    /*  u8 * d= Dest;*/

    while ( *Src )
    {
        /*__YRT    *d++ = (*Src)++;  */
        *Dest++ = *Src++;
    }
}



NODEBUG u8* DFS_strncpy( u8* s1, const u8* s2, s32 n )
{
    u8 c;
    u8* s = s1;

    --s1;

    if ( n >= 4 )
    {
        s32 n4 = n >> 2;

        for ( ;; )
        {
            c = *s2++;
            *++s1 = c;
            if ( c == '\0' )
                break;
            c = *s2++;
            *++s1 = c;
            if ( c == '\0' )
                break;
            c = *s2++;
            *++s1 = c;
            if ( c == '\0' )
                break;
            c = *s2++;
            *++s1 = c;
            if ( c == '\0' )
                break;
            if ( --n4 == 0 )
                goto last_chars;
        }
        n = n - ( s1 - s ) - 1;
        if ( n == 0 )
            return s;
        goto zero_fill;
    }

last_chars:
    n &= 3;
    if ( n == 0 )
        return s;

    do
    {
        c = *s2++;
        *++s1 = c;
        if ( --n == 0 )
            return s;
    }
    while ( c != '\0' );

zero_fill:
    do
        *++s1 = '\0';
    while ( --n > 0 );

    return s;
}

NODEBUG s32 DFS_strcmp( u8* s1,  u8* s2 )
{
    while ( *s1 == *s2 && *s1 != '\0' )
    {
        s1++;
        s2++;
    }

    /* Here we assume that characters are unsigned (0-255) */

    return *s1 - *s2;
}


/*===================================================================*/
/* User-supplied functions*/
NODEBUG uint32_t DFS_ReadSector( uint8_t unit, uint8_t* buffer, uint32_t sector, uint32_t count )
{

    u32* buffer32 = ( u32* )buffer;

#if !SDCARD_SDIO
    MSD_ReadBlock( buffer, sector << 9, SECTOR_SIZE );
    return 0;
#else
    SD_Error Status = SD_OK;
    Status = SD_ReadBlock( sector << 9 , buffer32, SECTOR_SIZE );

    if ( Status == SD_OK )
        return 0;
    else
        return 1;
#endif

}


NODEBUG uint32_t DFS_WriteSector( uint8_t unit, uint8_t* buffer, uint32_t sector, uint32_t count )
{
    u32	*buffer32 = ( u32* )buffer;

    /* Write block of 512 bytes on address 0 */
#if !SDCARD_SDIO
    MSD_WriteBlock( buffer, sector << 9, SECTOR_SIZE );
    return 0;
#else
    SD_Error Status = SD_OK;
    Status = SD_WriteBlock( sector << 9, buffer32, SECTOR_SIZE );

    if ( Status == SD_OK )
        return 0;
    else
        return 1;
#endif

}


/*===================================================================*/




/*
    Get starting sector# of specified partition on drive #unit
    NOTE: This code ASSUMES an MBR on the disk.
    scratchsector should point to a SECTOR_SIZE scratch area
    Returns 0xffffffff for any error.
    If pactive is non-NULL, this function also returns the partition active flag.
    If pptype is non-NULL, this function also returns the partition type.
    If psize is non-NULL, this function also returns the partition size.
*/
NODEBUG uint32_t DFS_GetPtnStart( uint8_t unit, uint8_t* scratchsector, uint8_t pnum, uint8_t* pactive, uint8_t* pptype, uint32_t* psize )
{
    uint32_t result;
    PMBR mbr = ( PMBR ) scratchsector;

    // DOS ptable supports maximum 4 partitions
    if ( pnum > 3 )
        return DFS_ERRMISC;

    // Read MBR from target media
    if ( DFS_ReadSector( unit, scratchsector, 0, 1 ) )
    {
        return DFS_ERRMISC;
    }

    result = ( uint32_t ) mbr->ptable[pnum].start_0 |
             ((( uint32_t ) mbr->ptable[pnum].start_1 ) << 8 ) |
             ((( uint32_t ) mbr->ptable[pnum].start_2 ) << 16 ) |
             ((( uint32_t ) mbr->ptable[pnum].start_3 ) << 24 );

    if ( pactive )
        *pactive = mbr->ptable[pnum].active;

    if ( pptype )
        *pptype = mbr->ptable[pnum].type;

    if ( psize )
        *psize = ( uint32_t ) mbr->ptable[pnum].size_0 |
                 ((( uint32_t ) mbr->ptable[pnum].size_1 ) << 8 ) |
                 ((( uint32_t ) mbr->ptable[pnum].size_2 ) << 16 ) |
                 ((( uint32_t ) mbr->ptable[pnum].size_3 ) << 24 );

    return result;
}


/*
    Retrieve volume info from BPB and store it in a VOLINFO structure
    You must provide the unit and starting sector of the filesystem, and
    a pointer to a sector buffer for scratch
    Attempts to read BPB and glean information about the FS from that.
    Returns 0 OK, nonzero for any error.
*/
NODEBUG uint32_t DFS_GetVolInfo( uint8_t unit, uint8_t* scratchsector, uint32_t startsector, PVOLINFO volinfo )
{
    PLBR lbr = ( PLBR ) scratchsector;
    volinfo->unit = unit;
    volinfo->startsector = startsector;

    if ( DFS_ReadSector( unit, scratchsector, startsector, 1 ) )
        return DFS_ERRMISC;

    /* tag: OEMID, refer dosfs.h*/
    /*  strncpy(volinfo->oemid, lbr->oemid, 8);*/
    /*  volinfo->oemid[8] = 0;*/

    volinfo->secperclus = lbr->bpb.secperclus;
    volinfo->reservedsecs = ( uint16_t ) lbr->bpb.reserved_l |
                            ((( uint16_t ) lbr->bpb.reserved_h ) << 8 );

    volinfo->numsecs = ( uint16_t ) lbr->bpb.sectors_s_l |
                       ((( uint16_t ) lbr->bpb.sectors_s_h ) << 8 );

    if ( !volinfo->numsecs )
        volinfo->numsecs = ( uint32_t ) lbr->bpb.sectors_l_0 |
                           ((( uint32_t ) lbr->bpb.sectors_l_1 ) << 8 ) |
                           ((( uint32_t ) lbr->bpb.sectors_l_2 ) << 16 ) |
                           ((( uint32_t ) lbr->bpb.sectors_l_3 ) << 24 );

    /* If secperfat is 0, we must be in a FAT32 volume; get secperfat*/
    /* from the FAT32 EBPB. The volume label and system ID string are also*/
    /* in different locations for FAT12/16 vs FAT32.*/
    volinfo->secperfat = ( uint16_t ) lbr->bpb.secperfat_l |
                         ((( uint16_t ) lbr->bpb.secperfat_h ) << 8 );
    if ( !volinfo->secperfat )
    {
        volinfo->secperfat = ( uint32_t ) lbr->ebpb.ebpb32.fatsize_0 |
                             ((( uint32_t ) lbr->ebpb.ebpb32.fatsize_1 ) << 8 ) |
                             ((( uint32_t ) lbr->ebpb.ebpb32.fatsize_2 ) << 16 ) |
                             ((( uint32_t ) lbr->ebpb.ebpb32.fatsize_3 ) << 24 );

        DFS_memcpy( volinfo->label, lbr->ebpb.ebpb32.label, 11 );
        volinfo->label[11] = 0;

        /* tag: OEMID, refer dosfs.h*/
        /*      DFS_memcpy(volinfo->system, lbr->ebpb.ebpb32.system, 8);*/
        /*      volinfo->system[8] = 0; */
    }
    else
    {
        DFS_memcpy( volinfo->label, lbr->ebpb.ebpb.label, 11 );
        volinfo->label[11] = 0;

        /* tag: OEMID, refer dosfs.h*/
        /*      DFS_memcpy(volinfo->system, lbr->ebpb.ebpb.system, 8);*/
        /*      volinfo->system[8] = 0; */
    }

    /* note: if rootentries is 0, we must be in a FAT32 volume.*/
    volinfo->rootentries = ( uint16_t ) lbr->bpb.rootentries_l |
                           ((( uint16_t ) lbr->bpb.rootentries_h ) << 8 );

    /* after extracting raw info we perform some useful precalculations*/
    volinfo->fat1 = startsector + volinfo->reservedsecs;

    /* The calculation below is designed to round up the root directory size for FAT12/16*/
    /* and to simply ignore the root directory for FAT32, since it's a normal, expandable*/
    /* file in that situation.*/
    if ( volinfo->rootentries )
    {
        volinfo->rootdir = volinfo->fat1 + ( volinfo->secperfat * 2 );
        volinfo->dataarea = volinfo->rootdir + ((( volinfo->rootentries * 32 ) + ( SECTOR_SIZE - 1 ) ) / SECTOR_SIZE );
    }
    else
    {
        volinfo->dataarea = volinfo->fat1 + ( volinfo->secperfat * 2 );
        volinfo->rootdir = ( uint32_t ) lbr->ebpb.ebpb32.root_0 |
                           ((( uint32_t ) lbr->ebpb.ebpb32.root_1 ) << 8 ) |
                           ((( uint32_t ) lbr->ebpb.ebpb32.root_2 ) << 16 ) |
                           ((( uint32_t ) lbr->ebpb.ebpb32.root_3 ) << 24 );
    }

    /* Calculate number of clusters in data area and infer FAT type from this information.*/
    volinfo->numclusters = ( volinfo->numsecs - volinfo->dataarea ) / volinfo->secperclus;
    if ( volinfo->numclusters < 4085 )
        volinfo->filesystem = FAT12;
    else if ( volinfo->numclusters < 65525 )
        volinfo->filesystem = FAT16;
    else
        volinfo->filesystem = FAT32;

    return DFS_OK;
}

/*
    Fetch FAT entry for specified cluster number
    You must provide a scratch buffer for one sector (SECTOR_SIZE) and a populated VOLINFO
    Returns a FAT32 BAD_CLUSTER value for any error, otherwise the contents of the desired
    FAT entry.
    scratchcache should point to a UINT32. This variable caches the physical sector number
    last read into the scratch buffer for performance enhancement reasons.
*/
NODEBUG uint32_t DFS_GetFAT( PVOLINFO volinfo, uint8_t* scratch, uint32_t* scratchcache, uint32_t cluster )
{
    uint32_t offset, sector, result;

    if ( volinfo->filesystem == FAT12 )
    {
        offset = cluster + ( cluster >> 2 );
    }
    else if ( volinfo->filesystem == FAT16 )
    {
        offset = cluster * 2;
    }
    else if ( volinfo->filesystem == FAT32 )
    {
        offset = cluster * 4;
    }
    else
        return 0x0ffffff7;  // FAT32 bad cluster

    /* at this point, offset is the BYTE offset of the desired sector from the start*/
    /* of the FAT. Calculate the physical sector containing this FAT entry.*/
    sector = offset / SECTOR_SIZE  + volinfo->fat1;

    /* If this is not the same sector we last read, then read it into RAM*/
    if ( sector != *scratchcache )
    {
        if ( DFS_ReadSector( volinfo->unit, scratch, sector, 1 ) )
        {
            /* avoid anyone assuming that this cache value is still valid, which*/
            /* might cause disk corruption*/
            *scratchcache = 0;
            return 0x0ffffff7;  /* FAT32 bad cluster    */
        }
        *scratchcache = sector;
    }

    /* At this point, we "merely" need to extract the relevant entry.*/
    /* This is easy for FAT16 and FAT32, but a royal PITA for FAT12 as a single entry*/
    /* may span a sector boundary. The normal way around this is always to read two*/
    /* FAT sectors, but that luxury is (by design intent) unavailable to DOSFS.*/
	offset = offset % SECTOR_SIZE;

    if ( volinfo->filesystem == FAT12 )
    {
        /* Special case for sector boundary - Store last byte of current sector.*/
        /* Then read in the next sector and put the first byte of that sector into*/
        /* the high byte of result.*/
        if ( offset == SECTOR_SIZE - 1 )
        {
            result = ( uint32_t ) scratch[offset];
            sector++;
            if ( DFS_ReadSector( volinfo->unit, scratch, sector, 1 ) )
            {
                /* avoid anyone assuming that this cache value is still valid, which*/
                /* might cause disk corruption*/
                *scratchcache = 0;
                return 0x0ffffff7;  /* FAT32 bad cluster    */
            }
            *scratchcache = sector;
            /* Thanks to Claudio Leonel for pointing out this missing line.*/
            result |= (( uint32_t ) scratch[0] ) << 8;
        }
        else
        {
            result = ( uint32_t ) scratch[offset] |
                     (( uint32_t ) scratch[offset+1] ) << 8;
        }
        if ( cluster & 1 )
            result = result >> 4;
        else
            result = result & 0xfff;
    }
    else if ( volinfo->filesystem == FAT16 )
    {
        result = ( uint32_t ) scratch[offset] |
                 (( uint32_t ) scratch[offset+1] ) << 8;
    }
    else if ( volinfo->filesystem == FAT32 )
    {
        result = (( uint32_t ) scratch[offset] |
                  (( uint32_t ) scratch[offset+1] ) << 8 |
                  (( uint32_t ) scratch[offset+2] ) << 16 |
                  (( uint32_t ) scratch[offset+3] ) << 24 ) & 0x0fffffff;
    }
    else
        result = 0x0ffffff7;    // FAT32 bad cluster
    return result;
}


/*
    Set FAT entry for specified cluster number
    You must provide a scratch buffer for one sector (SECTOR_SIZE) and a populated VOLINFO
    Returns DFS_ERRMISC for any error, otherwise DFS_OK
    scratchcache should point to a UINT32. This variable caches the physical sector number
    last read into the scratch buffer for performance enhancement reasons.

    NOTE: This code is HIGHLY WRITE-INEFFICIENT, particularly for flash media. Considerable
    performance gains can be realized by caching the sector. However this is difficult to
    achieve on FAT12 without requiring 2 sector buffers of scratch space, and it is a design
    requirement of this code to operate on a single 512-byte scratch.

    If you are operating DOSFS over flash, you are strongly advised to implement a writeback
    cache in your physical I/O driver. This will speed up your code significantly and will
    also conserve power and flash write life.
*/
NODEBUG uint32_t DFS_SetFAT( PVOLINFO volinfo, uint8_t* scratch, uint32_t* scratchcache, uint32_t cluster, uint32_t new_contents )
{
    uint32_t offset, sector, result;
    if ( volinfo->filesystem == FAT12 )
    {
        offset = cluster + ( cluster / 2 );
        new_contents &= 0xfff;
    }
    else if ( volinfo->filesystem == FAT16 )
    {
        offset = cluster * 2;
        new_contents &= 0xffff;
    }
    else if ( volinfo->filesystem == FAT32 )
    {
        offset = cluster * 4;
        new_contents &= 0x0fffffff; /* FAT32 is really "FAT28"*/
    }
    else
        return DFS_ERRMISC;

    /* at this point, offset is the BYTE offset of the desired sector from the start*/
    /* of the FAT. Calculate the physical sector containing this FAT entry.*/
    sector =  offset / SECTOR_SIZE + volinfo->fat1;

    /* If this is not the same sector we last read, then read it into RAM*/
    if ( sector != *scratchcache )
    {
        if ( DFS_ReadSector( volinfo->unit, scratch, sector, 1 ) )
        {
            /* avoid anyone assuming that this cache value is still valid, which*/
            /* might cause disk corruption*/
            *scratchcache = 0;
            return DFS_ERRMISC;
        }
        *scratchcache = sector;
    }

    /* At this point, we "merely" need to extract the relevant entry.*/
    /* This is easy for FAT16 and FAT32, but a royal PITA for FAT12 as a single entry*/
    /* may span a sector boundary. The normal way around this is always to read two*/
    /* FAT sectors, but that luxury is (by design intent) unavailable to DOSFS.*/
	offset = offset % SECTOR_SIZE;

    if ( volinfo->filesystem == FAT12 )
    {

        /* If this is an odd cluster, pre-shift the desired new contents 4 bits to*/
        /* make the calculations below simpler*/
        if ( cluster & 1 )
            new_contents = new_contents << 4;

        /* Special case for sector boundary*/
        if ( offset == SECTOR_SIZE - 1 )
        {

            /* Odd cluster: High 12 bits being set*/
            if ( cluster & 1 )
            {
				scratch[offset] = ( scratch[offset] & 0x0f ) | (new_contents & 0xf0);
            }
            /* Even cluster: Low 12 bits being set*/
            else
            {
                scratch[offset] = new_contents & 0xff;
            }
            result = DFS_WriteSector( volinfo->unit, scratch, *scratchcache, 1 );
            /* mirror the FAT into copy 2*/
            if ( DFS_OK == result )
                result = DFS_WriteSector( volinfo->unit, scratch, ( *scratchcache ) + volinfo->secperfat, 1 );

            /* If we wrote that sector OK, then read in the subsequent sector*/
            /* and poke the first byte with the remainder of this FAT entry.*/
            if ( DFS_OK == result )
            {
                *scratchcache++;
                result = DFS_ReadSector( volinfo->unit, scratch, *scratchcache, 1 );
                if ( DFS_OK == result )
                {
                    /* Odd cluster: High 12 bits being set*/
                    if ( cluster & 1 )
                    {
                        scratch[0] = new_contents & 0xff00;
                    }
                    /* Even cluster: Low 12 bits being set*/
                    else
                    {
						scratch[0] = ( scratch[0] & 0xf0 ) | (new_contents & 0x0f);
                    }
                    result = DFS_WriteSector( volinfo->unit, scratch, *scratchcache, 1 );
                    /* mirror the FAT into copy 2*/
                    if ( DFS_OK == result )
                        result = DFS_WriteSector( volinfo->unit, scratch, ( *scratchcache ) + volinfo->secperfat, 1 );
                }
                else
                {
                    /* avoid anyone assuming that this cache value is still valid, which*/
                    /* might cause disk corruption*/
                    *scratchcache = 0;
                }
            }
        } /* if (offset == SECTOR_SIZE - 1)*/

        /* Not a sector boundary. But we still have to worry about if it's an odd*/
        /* or even cluster number.*/
        else
        {
            /* Odd cluster: High 12 bits being set*/
            if ( cluster & 1 )
            {
				scratch[offset] = ( scratch[offset] & 0x0f ) | (new_contents & 0xf0);
                scratch[offset+1] = new_contents & 0xff00;
            }
            /* Even cluster: Low 12 bits being set*/
            else
            {
                scratch[offset] = new_contents & 0xff;
				scratch[offset+1] = ( scratch[offset+1] & 0xf0 ) | (new_contents & 0x0f);
            }
            result = DFS_WriteSector( volinfo->unit, scratch, *scratchcache, 1 );
            /* mirror the FAT into copy 2*/
            if ( DFS_OK == result )
                result = DFS_WriteSector( volinfo->unit, scratch, ( *scratchcache ) + volinfo->secperfat, 1 );
        }
    }
    else if ( volinfo->filesystem == FAT16 )
    {
        scratch[offset] = ( new_contents & 0xff );
        scratch[offset+1] = ( new_contents & 0xff00 ) >> 8;
        result = DFS_WriteSector( volinfo->unit, scratch, *scratchcache, 1 );
        /* mirror the FAT into copy 2*/
        if ( DFS_OK == result )
            result = DFS_WriteSector( volinfo->unit, scratch, ( *scratchcache ) + volinfo->secperfat, 1 );
    }
    else if ( volinfo->filesystem == FAT32 )
    {
        scratch[offset] = ( new_contents & 0xff );
        scratch[offset+1] = ( new_contents & 0xff00 ) >> 8;
        scratch[offset+2] = ( new_contents & 0xff0000 ) >> 16;
        scratch[offset+3] = ( scratch[offset+3] & 0xf0 ) | (( new_contents & 0x0f000000 ) >> 24 );
        /* Note well from the above: Per Microsoft's guidelines we preserve the upper*/
        /* 4 bits of the FAT32 cluster value. It's unclear what these bits will be used*/
        /* for; in every example I've encountered they are always zero.*/
        result = DFS_WriteSector( volinfo->unit, scratch, *scratchcache, 1 );
        /* mirror the FAT into copy 2*/
        if ( DFS_OK == result )
            result = DFS_WriteSector( volinfo->unit, scratch, ( *scratchcache ) + volinfo->secperfat, 1 );
    }
    else
        result = DFS_ERRMISC;

    return result;
}

/*
    Convert a filename element from canonical (8.3) to directory entry (11) form
    src must point to the first non-separator character.
    dest must point to a 12-byte buffer.
*/
NODEBUG uint8_t* DFS_CanonicalToDir( uint8_t* dest, uint8_t* src )
{
    uint8_t* destptr = dest;
    vs32 c = '5';

    c = *src;

    DFS_memset( dest, ' ', 11 );
    dest[11] = 0;

    while ( *src && ( *src != DIR_SEPARATOR ) && ( destptr - dest < 11 ) )
    {
        if ( *src >= 'a' && *src <= 'z' )
        {
            *destptr++ = c = ( *src - 'a' ) + 'A';
            src++;
        }
        else if ( *src == '.' )
        {
            src++;
            destptr = dest + 8;
        }
        else
        {
            *destptr = c;
            *destptr++ = *src++;
        }
    }

    return dest;
}

/*
    Find the first unused FAT entry
    You must provide a scratch buffer for one sector (SECTOR_SIZE) and a populated VOLINFO
    Returns a FAT32 BAD_CLUSTER value for any error, otherwise the contents of the desired
    FAT entry.
    Returns FAT32 bad_sector (0x0ffffff7) if there is no free cluster available
*/
NODEBUG uint32_t DFS_GetFreeFAT( PVOLINFO volinfo, uint8_t* scratch )
{
    uint32_t i, result = 0xffffffff, scratchcache = 0;

    /* Search starts at cluster 2, which is the first usable cluster*/
    /* NOTE: This search can't terminate at a bad cluster, because there might*/
    /* legitimately be bad clusters on the disk.*/
    for ( i = 2; i < volinfo->numclusters; i++ )
    {
        result = DFS_GetFAT( volinfo, scratch, &scratchcache, i );
        if ( !result )
        {
            return i;
        }
    }
    return 0x0ffffff7;      /* Can't find a free cluster*/
}


/*
    Open a directory for enumeration by DFS_GetNextDirEnt
    You must supply a populated VOLINFO (see DFS_GetVolInfo)
    The empty string or a string containing only the directory separator are
    considered to be the root directory.
    Returns 0 OK, nonzero for any error.
*/
NODEBUG uint32_t DFS_OpenDir( PVOLINFO volinfo, uint8_t* dirname, PDIRINFO dirinfo )
{
    /* Default behavior is a regular search for existing entries*/
    dirinfo->flags = 0;
    u8 c = *dirname;

    /*  if (!DFS_strlen((u8 *) dirname) || (DFS_strlen((u8 *) dirname) == 1 && dirname[0] == DIR_SEPARATOR))   // YRT20080204*/
    if ( !my_strlen(( u8* ) dirname ) || ( my_strlen(( u8* ) dirname ) == 1 && dirname[0] == DIR_SEPARATOR ) )
    {
        if ( volinfo->filesystem == FAT32 )
        {
            dirinfo->currentcluster = volinfo->rootdir;
            dirinfo->currentsector = 0;
            dirinfo->currententry = 0;

            /* read first sector of directory*/
            return DFS_ReadSector( volinfo->unit, dirinfo->scratch, volinfo->dataarea + (( volinfo->rootdir - 2 ) * volinfo->secperclus ), 1 );
        }
        else
        {
            dirinfo->currentcluster = 0;
            dirinfo->currentsector = 0;
            dirinfo->currententry = 0;

            /* read first sector of directory*/
            return DFS_ReadSector( volinfo->unit, dirinfo->scratch, volinfo->rootdir, 1 );
        }
    }

    /* This is not the root directory. We need to find the start of this subdirectory.*/
    /* We do this by devious means, using our own companion function DFS_GetNext.*/
    else
    {
        uint8_t tmpfn[12];
        uint8_t* ptr = dirname;
        uint32_t result;
        DIRENT de;

        if ( volinfo->filesystem == FAT32 )
        {
            dirinfo->currentcluster = volinfo->rootdir;
            dirinfo->currentsector = 0;
            dirinfo->currententry = 0;

            /* read first sector of directory*/
            if ( DFS_ReadSector( volinfo->unit, dirinfo->scratch, volinfo->dataarea + (( volinfo->rootdir - 2 ) * volinfo->secperclus ), 1 ) )
                return DFS_ERRMISC;
        }
        else
        {
            dirinfo->currentcluster = 0;
            dirinfo->currentsector = 0;
            dirinfo->currententry = 0;

            /* read first sector of directory*/
            if ( DFS_ReadSector( volinfo->unit, dirinfo->scratch, volinfo->rootdir, 1 ) )
                return DFS_ERRMISC;
        }

        /* skip leading path separators*/
        while ( *ptr == DIR_SEPARATOR && *ptr )
            ptr++;

        /* Scan the path from left to right, finding the start cluster of each entry*/
        /* Observe that this code is inelegant, but obviates the need for recursion.*/
        while ( *ptr )
        {

            DFS_CanonicalToDir( tmpfn, ptr );

            de.name[0] = 0;

            do
            {
                result = DFS_GetNext( volinfo, dirinfo, &de );
            }
            while ( !result && DFS_memcmp( de.name, tmpfn, 11 ) );

            if ( !DFS_memcmp( de.name, tmpfn, 11 ) && (( de.attr & ATTR_DIRECTORY ) == ATTR_DIRECTORY ) )
            {
                if ( volinfo->filesystem == FAT32 )
                {
                    dirinfo->currentcluster = ( uint32_t ) de.startclus_l_l |
                                              (( uint32_t ) de.startclus_l_h ) << 8 |
                                              (( uint32_t ) de.startclus_h_l ) << 16 |
                                              (( uint32_t ) de.startclus_h_h ) << 24;
                }
                else
                {
                    dirinfo->currentcluster = ( uint32_t ) de.startclus_l_l |
                                              (( uint32_t ) de.startclus_l_h ) << 8;
                }
                dirinfo->currentsector = 0;
                dirinfo->currententry = 0;

                if ( DFS_ReadSector( volinfo->unit, dirinfo->scratch, volinfo->dataarea + (( dirinfo->currentcluster - 2 ) * volinfo->secperclus ), 1 ) )
                    return DFS_ERRMISC;
            }
            else if ( !DFS_memcmp( de.name, tmpfn, 11 ) && !( de.attr & ATTR_DIRECTORY ) )
                return DFS_NOTFOUND;

            /* seek to next item in list*/
            while ( *ptr != DIR_SEPARATOR && *ptr )
                ptr++;
            if ( *ptr == DIR_SEPARATOR )
                ptr++;
        }

        if ( !dirinfo->currentcluster )
            return DFS_NOTFOUND;
    }
    return DFS_OK;
}

/*
    Get next entry in opened directory structure. Copies fields into the dirent
    structure, updates dirinfo. Note that it is the _caller's_ responsibility to
    handle the '.' and '..' entries.
    A deleted file will be returned as a NULL entry (first u8 of filename=0)
    by this code. Filenames beginning with 0x05 will be translated to 0xE5
    automatically. Long file name entries will be returned as NULL.
    returns DFS_EOF if there are no more entries, DFS_OK if this entry is valid,
    or DFS_ERRMISC for a media error
*/
NODEBUG uint32_t DFS_GetNext( PVOLINFO volinfo, PDIRINFO dirinfo, PDIRENT dirent )
{
    uint32_t tempint;    // required by DFS_GetFAT

    /* Do we need to read the next sector of the directory?*/
    if ( dirinfo->currententry >= SECTOR_SIZE / sizeof( DIRENT ) )
    {
        dirinfo->currententry = 0;
        dirinfo->currentsector++;

        /* Root directory; special case handling */
        /* Note that currentcluster will only ever be zero if both:*/
        /* (a) this is the root directory, and*/
        /* (b) we are on a FAT12/16 volume, where the root dir can't be expanded*/
        if ( dirinfo->currentcluster == 0 )
        {
            /* Trying to read past end of root directory?*/
            if ( dirinfo->currentsector *( SECTOR_SIZE / sizeof( DIRENT ) ) >= volinfo->rootentries )
                return DFS_EOF;

            /* Otherwise try to read the next sector*/
            if ( DFS_ReadSector( volinfo->unit, dirinfo->scratch, volinfo->rootdir + dirinfo->currentsector, 1 ) )
                return DFS_ERRMISC;
        }

        /* Normal handling*/
        else
        {
            if ( dirinfo->currentsector >= volinfo->secperclus )
            {
                dirinfo->currentsector = 0;
                if (( dirinfo->currentcluster >= 0xff7 &&  volinfo->filesystem == FAT12 ) ||
                        ( dirinfo->currentcluster >= 0xfff7 &&  volinfo->filesystem == FAT16 ) ||
                        ( dirinfo->currentcluster >= 0x0ffffff7 &&  volinfo->filesystem == FAT32 ) )
                {

                    /* We are at the end of the directory chain. If this is a normal*/
                    /* find operation, we should indicate that there is nothing more*/
                    /* to see.*/
                    if ( !( dirinfo->flags & DFS_DI_BLANKENT ) )
                        return DFS_EOF;

                    /* On the other hand, if this is a "find free entry" search,*/
                    /* we need to tell the caller to allocate a new cluster*/
                    else
                        return DFS_ALLOCNEW;
                }
                dirinfo->currentcluster = DFS_GetFAT( volinfo, dirinfo->scratch, &tempint, dirinfo->currentcluster );
            }
            if ( DFS_ReadSector( volinfo->unit, dirinfo->scratch, volinfo->dataarea + (( dirinfo->currentcluster - 2 ) * volinfo->secperclus ) + dirinfo->currentsector, 1 ) )
                return DFS_ERRMISC;
        }
    }

    DFS_memcpy( dirent, &((( PDIRENT ) dirinfo->scratch )[dirinfo->currententry] ), sizeof( DIRENT ) );

    if ( dirent->name[0] == 0 )        /* no more files in this directory*/
    {
        /* If this is a "find blank" then we can reuse this name.*/
        if ( dirinfo->flags & DFS_DI_BLANKENT )
        {
            //NTRF: Invalid indexing! Current must be incremented proir to returning OK
            dirinfo->currententry++;
            return DFS_OK;
        }
        else
            return DFS_EOF;
    }

    if ( dirent->name[0] == 0xe5 )  /* handle deleted file entries*/
        dirent->name[0] = 0;
    else if ((( dirent->attr & ATTR_LONG_NAME ) == ATTR_LONG_NAME ) &&
             !( dirinfo->flags & DFS_DI_BLANKENT ) ) //NTRF: don't replace longnames with new files
        dirent->name[0] = 0;
    else if ( dirent->name[0] == 0x05 )  /* handle kanji filenames beginning with 0xE5*/
        dirent->name[0] = 0xe5;

    dirinfo->currententry++;

    return DFS_OK;
}

/*
    INTERNAL
    Find a free directory entry in the directory specified by path
    This function MAY cause a disk write if it is necessary to extend the directory
    size.
    Note - di.scratch must be preinitialized to point to a sector scratch buffer
    de is a scratch structure
    Returns DFS_ERRMISC if a new entry could not be located or created
    de is updated with the same return information you would expect from DFS_GetNext
*/
NODEBUG uint32_t DFS_GetFreeDirEnt( PVOLINFO volinfo, uint8_t* path, PDIRINFO di, PDIRENT de )
{
    uint32_t tempclus, i;

    if ( DFS_OpenDir( volinfo, path, di ) )
        return DFS_NOTFOUND;

    /* Set "search for empty" flag so DFS_GetNext knows what we're doing*/
    di->flags |= DFS_DI_BLANKENT;

    /* We seek through the directory looking for an empty entry*/
    /* Note we are reusing tempclus as a temporary result holder.*/
    tempclus = 0;
    do
    {
        tempclus = DFS_GetNext( volinfo, di, de );

        /* Empty entry found*/
        if ( tempclus == DFS_OK && ( !de->name[0] ) )
        {
            return DFS_OK;
        }

        /* End of root directory reached*/
        else if ( tempclus == DFS_EOF )
            return DFS_ERRMISC;

        else if ( tempclus == DFS_ALLOCNEW )
        {
            tempclus = DFS_GetFreeFAT( volinfo, di->scratch );
            if ( tempclus == 0x0ffffff7 )
                return DFS_ERRMISC;

            /* write out zeroed sectors to the new cluster*/
            DFS_memset( di->scratch, 0, SECTOR_SIZE );
            for ( i = 0; i < volinfo->secperclus; i++ )
            {
                if ( DFS_WriteSector( volinfo->unit, di->scratch, volinfo->dataarea + (( tempclus - 2 ) * volinfo->secperclus ) + i, 1 ) )
                    return DFS_ERRMISC;
            }
            /* Point old end cluster to newly allocated cluster*/
            i = 0;
            DFS_SetFAT( volinfo, di->scratch, &i, di->currentcluster, tempclus );

            /* Update DIRINFO so caller knows where to place the new file           */
            di->currentcluster = tempclus;
            di->currentsector = 0;
            di->currententry = 1;   /* since the code coming after this expects to subtract 1*/

            /* Mark newly allocated cluster as end of chain         */
            switch ( volinfo->filesystem )
            {
            case FAT12:     tempclus = 0xff8;   break;
            case FAT16:     tempclus = 0xfff8;  break;
            case FAT32:     tempclus = 0x0ffffff8;  break;
            default:        return DFS_ERRMISC;
            }
            DFS_SetFAT( volinfo, di->scratch, &i, di->currentcluster, tempclus );
        }
    }
    while ( !tempclus );

    /* We shouldn't get here*/
    return DFS_ERRMISC;
}

/*
    Open a file for reading or writing. You supply populated VOLINFO, a path to the file,
    mode (DFS_READ or DFS_WRITE) and an empty fileinfo structure. You also need to
    provide a pointer to a sector-sized scratch buffer.
    Returns various DFS_* error states. If the result is DFS_OK, fileinfo can be used
    to access the file from this point on.
*/
NODEBUG uint32_t DFS_OpenFile( PVOLINFO volinfo, uint8_t* path, uint8_t mode, uint8_t* scratch, PFILEINFO fileinfo )
{
    uint8_t tmppath[MAX_PATH];
    uint8_t filename[12];
    uint8_t* p;
    DIRINFO di;
    DIRENT de;
    uint32_t dircluster;

    /* larwe 2006-09-16 +1 zero out file structure*/
    DFS_memset( fileinfo, 0, sizeof( FILEINFO ) );

    /* save access mode*/
    fileinfo->mode = mode;

    /* Get a local copy of the path. If it's longer than MAX_PATH, abort.*/
    DFS_strncpy(( u8* ) tmppath, ( u8* ) path, MAX_PATH );
    tmppath[MAX_PATH - 1] = 0;
    if ( DFS_strcmp(( u8* ) path, ( u8* ) tmppath ) )
    {
        return DFS_PATHLEN;
    }

    /* strip leading path separators*/
    while ( tmppath[0] == DIR_SEPARATOR )
        DFS_strcpy(( u8* ) tmppath, ( u8* ) tmppath + 1 );

    /* Parse filename off the end of the supplied path*/
    p = tmppath;
    while ( *( p++ ) );

    p--;
    while ( p > tmppath && *p != DIR_SEPARATOR ) /* larwe 9/16/06 ">=" to ">" bugfix*/
        p--;
    if ( *p == DIR_SEPARATOR )
        p++;

    DFS_CanonicalToDir( filename, p );

    if ( p > tmppath )
        p--;
    if ( *p == DIR_SEPARATOR || p == tmppath ) // larwe 9/16/06 +"|| p == tmppath" bugfix
        *p = 0;

    /* At this point, if our path was MYDIR/MYDIR2/FILE.EXT, filename = "FILE    EXT" and*/
    /* tmppath = "MYDIR/MYDIR2".*/
    di.scratch = scratch;
    dircluster = di.currentcluster;

    if ( DFS_OpenDir( volinfo, tmppath, &di ) )
        return DFS_NOTFOUND;

    while ( !DFS_GetNext( volinfo, &di, &de ) )
    {
        if ( !DFS_memcmp( de.name, filename, 11 ) )
        {
            /* You can't use this function call to open a directory.*/
            //NTRF: allows to create directories
            if (( de.attr & ATTR_DIRECTORY ) && ( mode != DFS_CREATEDIR ) )
                return DFS_NOTFOUND;

            fileinfo->volinfo = volinfo;
            fileinfo->pointer = 0;
            /* The reason we store this extra info about the file is so that we can*/
            /* speedily update the file size, modification date, etc. on a file that is*/
            /* opened for writing.*/
            if ( di.currentcluster == 0 )
                fileinfo->dirsector = volinfo->rootdir + di.currentsector;
            else
                fileinfo->dirsector = volinfo->dataarea + (( di.currentcluster - 2 ) * volinfo->secperclus ) + di.currentsector;
            fileinfo->diroffset = di.currententry - 1;
            if ( volinfo->filesystem == FAT32 )
            {
                fileinfo->cluster = ( uint32_t ) de.startclus_l_l |
                                    (( uint32_t ) de.startclus_l_h ) << 8 |
                                    (( uint32_t ) de.startclus_h_l ) << 16 |
                                    (( uint32_t ) de.startclus_h_h ) << 24;
            }
            else
            {
                fileinfo->cluster = ( uint32_t ) de.startclus_l_l |
                                    (( uint32_t ) de.startclus_l_h ) << 8;
            }
            fileinfo->firstcluster = fileinfo->cluster;
            fileinfo->filelen = ( uint32_t ) de.filesize_0 |
                                (( uint32_t ) de.filesize_1 ) << 8 |
                                (( uint32_t ) de.filesize_2 ) << 16 |
                                (( uint32_t ) de.filesize_3 ) << 24;

            return DFS_OK;
        }
    }

    /* At this point, we KNOW the file does not exist. If the file was opened*/
    /* with write access, we can create it.*/
    if ( mode & DFS_WRITE )
    {
        uint32_t cluster, temp;

        /* Locate or create a directory entry for this file*/
        if ( DFS_OK != DFS_GetFreeDirEnt( volinfo, tmppath, &di, &de ) )
            return DFS_ERRMISC;

        /* put sane values in the directory entry*/
        DFS_memset( &de, 0, sizeof( de ) );
        DFS_memcpy( de.name, filename, 11 );
        de.crttime_l = 0x20;    /* 01:01:00am, Jan 1, 2006.*/
        de.crttime_h = 0x08;
        de.crtdate_l = 0x11;
        de.crtdate_h = 0x34;
        de.lstaccdate_l = 0x11;
        de.lstaccdate_h = 0x34;
        de.wrttime_l = 0x20;
        de.wrttime_h = 0x08;
        de.wrtdate_l = 0x11;
        de.wrtdate_h = 0x34;

        //NTRF: create directory
        if ( mode == DFS_CREATEDIR ) de.attr |= ATTR_DIRECTORY;

        /* allocate a starting cluster for the directory entry*/
        cluster = DFS_GetFreeFAT( volinfo, scratch );

        de.startclus_l_l = cluster & 0xff;
        de.startclus_l_h = ( cluster & 0xff00 ) >> 8;
        de.startclus_h_l = ( cluster & 0xff0000 ) >> 16;
        de.startclus_h_h = ( cluster & 0xff000000 ) >> 24;

        /* update FILEINFO for our caller's sake*/
        fileinfo->volinfo = volinfo;
        fileinfo->pointer = 0;
        /* The reason we store this extra info about the file is so that we can*/
        /* speedily update the file size, modification date, etc. on a file that is*/
        /* opened for writing.*/
        if ( di.currentcluster == 0 )
            fileinfo->dirsector = volinfo->rootdir + di.currentsector;
        else
            fileinfo->dirsector = volinfo->dataarea + (( di.currentcluster - 2 ) * volinfo->secperclus ) + di.currentsector;
        fileinfo->diroffset = di.currententry - 1;
        fileinfo->cluster = cluster;
        fileinfo->firstcluster = cluster;
        fileinfo->filelen = 0;

        /* write the directory entry*/
        /* note that we no longer have the sector containing the directory entry,*/
        /* tragically, so we have to re-read it*/
        if ( DFS_ReadSector( volinfo->unit, scratch, fileinfo->dirsector, 1 ) )
            return DFS_ERRMISC;
        DFS_memcpy( &((( PDIRENT ) scratch )[di.currententry-1] ), &de, sizeof( DIRENT ) );
        if ( DFS_WriteSector( volinfo->unit, scratch, fileinfo->dirsector, 1 ) )
            return DFS_ERRMISC;

        /* Mark newly allocated cluster as end of chain            */
        switch ( volinfo->filesystem )
        {
        case FAT12:        cluster = 0xff8;    break;
        case FAT16:        cluster = 0xfff8;    break;
        case FAT32:        cluster = 0x0ffffff8;    break;
        default:        return DFS_ERRMISC;
        }
        temp = 0;
        DFS_SetFAT( volinfo, scratch, &temp, fileinfo->cluster, cluster );

        //Fill new directory with required info
        if ( mode == DFS_CREATEDIR )
        {
            if ( dircluster <= 2 ) //Root
                dircluster = 0;

            uint32_t startsector = volinfo->dataarea + (( fileinfo->cluster - 2 ) * volinfo->secperclus );
            uint32_t endsector = startsector + volinfo->secperclus - 1;

            DFS_memset( scratch, 0, SECTOR_SIZE );

            for ( ; endsector > startsector; endsector -= 1 )
                DFS_WriteSector( volinfo->unit, scratch, endsector, 1 );

            DFS_memcpy( &( de.name ), ".          ", 11 );
            de.filesize_0 = 0;
            de.filesize_1 = 0;
            de.filesize_2 = 0;
            de.filesize_3 = 0;
            DFS_memcpy( scratch, &de, sizeof( DIRENT ) );

            DFS_memcpy( &( de.name ), "..         ", 11 );
            de.startclus_l_l = dircluster & 0xff;
            de.startclus_l_h = ( dircluster & 0xff00 ) >> 8;
            de.startclus_h_l = ( dircluster & 0xff0000 ) >> 16;
            de.startclus_h_h = ( dircluster & 0xff000000 ) >> 24;

            DFS_memcpy( scratch + sizeof( DIRENT ), &de, sizeof( DIRENT ) );

            DFS_WriteSector( volinfo->unit, scratch, startsector, 1 );
        }

        return DFS_OK;
    }

    return DFS_NOTFOUND;
}


/*
    Read an open file
    You must supply a prepopulated FILEINFO as provided by DFS_OpenFile, and a
    pointer to a SECTOR_SIZE scratch buffer.
    Note that returning DFS_EOF is not an error condition. This function updates the
    successcount field with the number of bytes actually read.
*/
NODEBUG uint32_t DFS_ReadFile( PFILEINFO fileinfo, uint8_t* scratch, uint8_t* buffer, uint32_t* successcount, uint32_t len )
{
    uint32_t remain;
    uint32_t result = DFS_OK;
    uint32_t sector;
    uint32_t bytesread;

    /* Don't try to read past EOF*/
    if ( len > fileinfo->filelen - fileinfo->pointer )
        len = fileinfo->filelen - fileinfo->pointer;

    remain = len;
    *successcount = 0;

    while ( remain && result == DFS_OK )
    {
        /* This is a bit complicated. The sector we want to read is addressed at a cluster*/
        /* granularity by the fileinfo->cluster member. The file pointer tells us how many*/
        /* extra sectors to add to that number.*/
        sector = fileinfo->volinfo->dataarea +
                 (( fileinfo->cluster - 2 ) * fileinfo->volinfo->secperclus ) +
                   (fileinfo->pointer % (fileinfo->volinfo->secperclus * SECTOR_SIZE )) / SECTOR_SIZE;
	
	
        /* Case 1 - File pointer is not on a sector boundary*/
		if ( fileinfo->pointer % SECTOR_SIZE )
        {
            uint16_t tempreadsize;

            /* We always have to go through scratch in this case*/
            result = DFS_ReadSector( fileinfo->volinfo->unit, scratch, sector, 1 );

            /* This is the number of bytes that we actually care about in the sector*/
            /* just read.*/
			tempreadsize = SECTOR_SIZE - fileinfo->pointer % SECTOR_SIZE ;

            /* Case 1A - We want the entire remainder of the sector. After this*/
            /* point, all passes through the read loop will be aligned on a sector*/
            /* boundary, which allows us to go through the optimal path 2A below.*/
            if ( remain >= tempreadsize )
            {
                DFS_memcpy( buffer, scratch + ( SECTOR_SIZE - tempreadsize ), tempreadsize );
                bytesread = tempreadsize;
                buffer += tempreadsize;
                fileinfo->pointer += tempreadsize;
                remain -= tempreadsize;
            }
            /* Case 1B - This read concludes the file read operation*/
            else
            {
                DFS_memcpy( buffer, scratch + ( SECTOR_SIZE - tempreadsize ), remain );

                buffer += remain;
                fileinfo->pointer += remain;
                bytesread = remain;
                remain = 0;
            }
        }
        /* Case 2 - File pointer is on sector boundary*/
        else
        {
            /* Case 2A - We have at least one more full sector to read and don't have*/
            /* to go through the scratch buffer. You could insert optimizations here to*/
            /* read multiple sectors at a time, if you were thus inclined (note that*/
            /* the maximum multi-read you could perform is a single cluster, so it would*/
            /* be advantageous to have code similar to case 1A above that would round the*/
            /* pointer to a cluster boundary the first pass through, so all subsequent*/
            /* [large] read requests would be able to go a cluster at a time).*/
            if ( remain >= SECTOR_SIZE )
            {
                result = DFS_ReadSector( fileinfo->volinfo->unit, scratch, sector, 1 );
				DFS_memcpy( buffer, scratch, SECTOR_SIZE );
                remain -= SECTOR_SIZE;
                buffer += SECTOR_SIZE;
                fileinfo->pointer += SECTOR_SIZE;
                bytesread = SECTOR_SIZE;
            }
            /* Case 2B - We are only reading a partial sector*/
            else
            {
                result = DFS_ReadSector( fileinfo->volinfo->unit, scratch, sector, 1 );
                DFS_memcpy( buffer, scratch, remain );
                buffer += remain;
                fileinfo->pointer += remain;
                bytesread = remain;
                remain = 0;
            }
        }

        *successcount += bytesread;

        /* check to see if we stepped over a cluster boundary*/
	    if ( ( fileinfo->pointer - bytesread) / (fileinfo->volinfo->secperclus * SECTOR_SIZE ) !=
               fileinfo->pointer / (fileinfo->volinfo->secperclus * SECTOR_SIZE ) )
	    {
            /* An act of minor evil - we use bytesread as a scratch integer, knowing that*/
            /* its value is not used after updating *successcount above*/
            bytesread = 0;
            if ((( fileinfo->volinfo->filesystem == FAT12 ) && ( fileinfo->cluster >= 0xff8 ) ) ||
                    (( fileinfo->volinfo->filesystem == FAT16 ) && ( fileinfo->cluster >= 0xfff8 ) ) ||
                    (( fileinfo->volinfo->filesystem == FAT32 ) && ( fileinfo->cluster >= 0x0ffffff8 ) ) )
                result = DFS_EOF;
            else
                fileinfo->cluster = DFS_GetFAT( fileinfo->volinfo, scratch, &bytesread, fileinfo->cluster );
        }
    }

    return result;
}

/*
    Seek file pointer to a given position
    This function does not return status - refer to the fileinfo->pointer value
    to see where the pointer wound up.
    Requires a SECTOR_SIZE scratch buffer
*/
NODEBUG void DFS_Seek( PFILEINFO fileinfo, uint32_t offset, uint8_t* scratch )
{
    uint32_t tempint;

    /* larwe 9/16/06 bugfix split case 0a/0b and changed fallthrough handling*/
    /* Case 0a - Return immediately for degenerate case*/
    if ( offset == fileinfo->pointer )
    {
        return;
    }

    /* Case 0b - Don't allow the user to seek past the end of the file*/
    if ( offset > fileinfo->filelen )
    {
        offset = fileinfo->filelen;
        // NOTE NO RETURN HERE!
    }

    /* Case 1 - Simple rewind to start*/
    /* Note _intentional_ fallthrough from Case 0b above*/
    if ( offset == 0 )
    {
        fileinfo->cluster = fileinfo->firstcluster;
        fileinfo->pointer = 0;
        return;     /* larwe 9/16/06 +1 bugfix*/
    }
    /* Case 2 - Seeking backwards. Need to reset and seek forwards*/
    else if ( offset < fileinfo->pointer )
    {
        fileinfo->cluster = fileinfo->firstcluster;
        fileinfo->pointer = 0;
        /* NOTE NO RETURN HERE!*/
    }

    /* Case 3 - Seeking forwards*/
    /* Note _intentional_ fallthrough from Case 2 above*/

    /* Case 3a - Seek size does not cross cluster boundary - */
    /* very simple case*/
    /* larwe 9/16/06 changed .rem to .quot in both div calls, bugfix*/
    if (  fileinfo->pointer / (fileinfo->volinfo->secperclus * SECTOR_SIZE ) ==
        ( fileinfo->pointer + offset) / (fileinfo->volinfo->secperclus * SECTOR_SIZE ))
    {
        fileinfo->pointer = offset;
    }
    /* Case 3b - Seeking across cluster boundary(ies)*/
    else
    {
        /* round file pointer down to cluster boundary*/
		fileinfo->pointer =  (fileinfo->pointer / (fileinfo->volinfo->secperclus * SECTOR_SIZE )) *
        !                    fileinfo->volinfo->secperclus * SECTOR_SIZE;
        /* seek by clusters*/
        /* larwe 9/30/06 bugfix changed .rem to .quot in both div calls*/
        while (  fileinfo->pointer / (fileinfo->volinfo->secperclus * SECTOR_SIZE ) !=
                 offset / (fileinfo->volinfo->secperclus * SECTOR_SIZE ) )  //correct calculation of actual cluster
        {
            /* DFS_div(fileinfo->pointer + offset, fileinfo->volinfo->secperclus * SECTOR_SIZE).quot) {*/
            /* if you want to set the filepointer  beyond a cluster limit this line crashes the whole system*/
            /* adding the offset to the filepointer causes the while to loop infinitely */


            fileinfo->cluster = DFS_GetFAT( fileinfo->volinfo, scratch, &tempint, fileinfo->cluster );
            /* Abort if there was an error*/
            if ( fileinfo->cluster == 0x0ffffff7 )
            {
                fileinfo->pointer = 0;
                fileinfo->cluster = fileinfo->firstcluster;
                return;
            }
            fileinfo->pointer += SECTOR_SIZE * fileinfo->volinfo->secperclus;
        }

        /* since we know the cluster is right, we have no more work to do*/
        fileinfo->pointer = offset;
    }
}

/*
    Delete a file
    scratch must point to a sector-sized buffer
*/
NODEBUG uint32_t DFS_UnlinkFile( PVOLINFO volinfo, uint8_t* path, uint8_t* scratch )
{
    PDIRENT de = ( PDIRENT ) scratch;
    FILEINFO fi;
    uint32_t cache = 0;
    uint32_t tempclus;

    /* DFS_OpenFile gives us all the information we need to delete it*/
    if ( DFS_OK != DFS_OpenFile( volinfo, path, DFS_READ, scratch, &fi ) )
        return DFS_NOTFOUND;

    /* First, read the directory sector and delete that entry*/
    if ( DFS_ReadSector( volinfo->unit, scratch, fi.dirsector, 1 ) )
        return DFS_ERRMISC;
    (( PDIRENT ) scratch )[fi.diroffset].name[0] = 0xe5;
    if ( DFS_WriteSector( volinfo->unit, scratch, fi.dirsector, 1 ) )
        return DFS_ERRMISC;

    /* Now follow the cluster chain to free the file space*/
    while ( !(( volinfo->filesystem == FAT12 && fi.firstcluster >= 0x0ff7 ) ||
              ( volinfo->filesystem == FAT16 && fi.firstcluster >= 0xfff7 ) ||
              ( volinfo->filesystem == FAT32 && fi.firstcluster >= 0x0ffffff7 ) ) )
    {
        tempclus = fi.firstcluster;

        fi.firstcluster = DFS_GetFAT( volinfo, scratch, &cache, fi.firstcluster );
        DFS_SetFAT( volinfo, scratch, &cache, tempclus, 0 );

    }
    return DFS_OK;
}


/*
    Write an open file
    You must supply a prepopulated FILEINFO as provided by DFS_OpenFile, and a
    pointer to a SECTOR_SIZE scratch buffer.
    This function updates the successcount field with the number of bytes actually written.
*/
NODEBUG uint32_t DFS_WriteFile( PFILEINFO fileinfo, uint8_t* scratch, uint8_t* buffer, uint32_t* successcount, uint32_t len )
{
    uint32_t remain;
    uint32_t result = DFS_OK;
    uint32_t sector;
    uint32_t byteswritten;

    /* Don't allow writes to a file that's open as readonly*/
    if ( !( fileinfo->mode & DFS_WRITE ) )
        return DFS_ERRMISC;

    remain = len;
    *successcount = 0;

    while ( remain && result == DFS_OK )
    {
        /* This is a bit complicated. The sector we want to read is addressed at a cluster*/
        /* granularity by the fileinfo->cluster member. The file pointer tells us how many*/
        /* extra sectors to add to that number.*/
        sector = fileinfo->volinfo->dataarea +
                 (( fileinfo->cluster - 2 ) * fileinfo->volinfo->secperclus ) +
                  ( fileinfo->pointer % (fileinfo->volinfo->secperclus * SECTOR_SIZE )) / SECTOR_SIZE;

        /* Case 1 - File pointer is not on a sector boundary*/
	    uint16_t occupiedsize = fileinfo->pointer % SECTOR_SIZE ;
		if ( occupiedsize )
        {
        
            /* We always have to go through scratch in this case*/
            result = DFS_ReadSector( fileinfo->volinfo->unit, scratch, sector, 1 );

			/* Case 1A - We are writing the entire remainder of the sector. After*/
            /* this point, all passes through the read loop will be aligned on a*/
            /* sector boundary, which allows us to go through the optimal path*/
            /* 2A below.*/
            if ( remain >= SECTOR_SIZE - occupiedsize )
            {
				DFS_memcpy( scratch + occupiedsize, buffer, SECTOR_SIZE - occupiedsize );
                if ( !result )
                    result = DFS_WriteSector( fileinfo->volinfo->unit, scratch, sector, 1 );
							
                byteswritten = SECTOR_SIZE - occupiedsize;
            }
            /* Case 1B - This concludes the file write operation*/
            else
            {
				DFS_memcpy( scratch + occupiedsize, buffer, remain );
                if ( !result )
                    result = DFS_WriteSector( fileinfo->volinfo->unit, scratch, sector, 1 );
							
				byteswritten = remain;
            }
			remain -= byteswritten;	
			buffer += byteswritten;
			fileinfo->pointer += byteswritten;
			if ( fileinfo->filelen < fileinfo->pointer )
				fileinfo->filelen = fileinfo->pointer;
            
        } /* case 1*/
        /* Case 2 - File pointer is on sector boundary*/
        else
        {
            /* Case 2A - We have at least one more full sector to write and don't have*/
            /* to go through the scratch buffer. You could insert optimizations here to*/
            /* write multiple sectors at a time, if you were thus inclined. Refer to*/
            /* similar notes in DFS_ReadFile.*/
            if ( remain >= SECTOR_SIZE )
            {
				DFS_memcpy( scratch, buffer, SECTOR_SIZE );
				result = DFS_WriteSector( fileinfo->volinfo->unit, scratch, sector, 1 );
                byteswritten = SECTOR_SIZE;
            }
            /* Case 2B - We are only writing a partial sector and potentially need to*/
            /* go through the scratch buffer.*/
            else
            {
                /* If the current file pointer is not yet at or beyond the file*/
                /* length, we are writing somewhere in the middle of the file and*/
                /* need to load the original sector to do a read-modify-write.*/
                if ( fileinfo->pointer < fileinfo->filelen )
                {
					result = DFS_ReadSector( fileinfo->volinfo->unit, scratch, sector, 1 );
                    if ( !result )
                    {
                        DFS_memcpy( scratch, buffer, remain );
                        result = DFS_WriteSector( fileinfo->volinfo->unit, scratch, sector, 1 );
                    }
                }
                else
                {
                    DFS_memcpy( scratch, buffer, remain );
					result = DFS_WriteSector( fileinfo->volinfo->unit, scratch, sector, 1 );
                }

                byteswritten = remain;
            }
			remain -= byteswritten;
			buffer += byteswritten;
			fileinfo->pointer += byteswritten;
			if ( fileinfo->filelen < fileinfo->pointer )
				fileinfo->filelen = fileinfo->pointer;
        }

        *successcount += byteswritten;

        /* check to see if we stepped over a cluster boundary*/
        if ( ( fileinfo->pointer - byteswritten) /  (fileinfo->volinfo->secperclus * SECTOR_SIZE ) !=
               fileinfo->pointer / (fileinfo->volinfo->secperclus * SECTOR_SIZE ))
		{
            uint32_t lastcluster;

            /* We've transgressed into another cluster. If we were already at EOF,*/
            /* we need to allocate a new cluster.*/
            /* An act of minor evil - we use byteswritten as a scratch integer, knowing*/
            /* that its value is not used after updating *successcount above*/
            byteswritten = 0;

            lastcluster = fileinfo->cluster;
            fileinfo->cluster = DFS_GetFAT( fileinfo->volinfo, scratch, &byteswritten, fileinfo->cluster );

            /* Allocate a new cluster?*/
            if ((( fileinfo->volinfo->filesystem == FAT12 ) && ( fileinfo->cluster >= 0xff8 ) ) ||
                    (( fileinfo->volinfo->filesystem == FAT16 ) && ( fileinfo->cluster >= 0xfff8 ) ) ||
                    (( fileinfo->volinfo->filesystem == FAT32 ) && ( fileinfo->cluster >= 0x0ffffff8 ) ) )
            {
                uint32_t tempclus;

                tempclus = DFS_GetFreeFAT( fileinfo->volinfo, scratch );
                byteswritten = 0; /* invalidate cache*/
                if ( tempclus == 0x0ffffff7 )
                    return DFS_ERRMISC;

                /* Link new cluster onto file*/
                DFS_SetFAT( fileinfo->volinfo, scratch, &byteswritten, lastcluster, tempclus );
                fileinfo->cluster = tempclus;

                /* Mark newly allocated cluster as end of chain         */
                switch ( fileinfo->volinfo->filesystem )
                {
                case FAT12:     tempclus = 0xff8;   break;
                case FAT16:     tempclus = 0xfff8;  break;
                case FAT32:     tempclus = 0x0ffffff8;  break;
                default:        return DFS_ERRMISC;
                }
                DFS_SetFAT( fileinfo->volinfo, scratch, &byteswritten, fileinfo->cluster, tempclus );

                result = DFS_OK;
            }
            /* No else clause is required.*/
        }
    }

    /* Update directory entry*/
    if ( DFS_ReadSector( fileinfo->volinfo->unit, scratch, fileinfo->dirsector, 1 ) )
        return DFS_ERRMISC;
    (( PDIRENT ) scratch )[fileinfo->diroffset].filesize_0 = fileinfo->filelen & 0xff;
    (( PDIRENT ) scratch )[fileinfo->diroffset].filesize_1 = ( fileinfo->filelen & 0xff00 ) >> 8;
    (( PDIRENT ) scratch )[fileinfo->diroffset].filesize_2 = ( fileinfo->filelen & 0xff0000 ) >> 16;
    (( PDIRENT ) scratch )[fileinfo->diroffset].filesize_3 = ( fileinfo->filelen & 0xff000000 ) >> 24;
    if ( DFS_WriteSector( fileinfo->volinfo->unit, scratch, fileinfo->dirsector, 1 ) )
        return DFS_ERRMISC;
    return result;
}

NODEBUG u32 DFS_Mount( enum STORAGE_device device )
{
    uint8_t sector[SECTOR_SIZE];
    uint32_t pstart, psize;
    uint8_t pactive, ptype;

    switch ( device )
    {
    case MMCSD_SDIO:
    {
        /*///////////////////////////////////////////////////////////////////*/
        /*////// SDCARD Initialisation //////////////////////////////////////*/
        /*///////////////Section adapted from ST example/////////////////////*/

#if !SDCARD_SDIO
        /*-------------------------- SD Init ----------------------------- */
        if ( MSD_Init() == MSD_RESPONSE_FAILURE )
            return -1;

#else
        /*-------------------------- SD Init ----------------------------- */
        Status = SD_Init();

        if ( Status == SD_OK )
        {
            /*----------------- Read CSD/CID MSD registers ------------------*/
            Status = SD_GetCardInfo( &SDCardInfo );
        }

        if ( Status == SD_OK )
        {
            /*----------------- Select Card --------------------------------*/
            Status = SD_SelectDeselect(( u32 )( SDCardInfo.RCA << 16 ) );
        }

        if ( Status == SD_OK )
        {
            Status = SD_EnableWideBusOperation( SDIO_BusWide_4b );
        }

        /* Set Device Transfer Mode to DMA */
        if ( Status == SD_OK )
        {
            Status = SD_SetDeviceMode( SD_DMA_MODE );
        }

        if ( Status != SD_OK )
            return -1;

#endif  // SDCARD_SDIO

        if ( DFS_ReadSector( 0, sector, 0, 1 ) )
        {
            /*  Cannot read media!*/
            return -1;
        }

        if (( sector[0x1FE] == 0x55 ) & ( sector[0x1FF] == 0xAA ) )
        {
            /* The end of the last read sector contains a FAT or MBR end delimiter*/
            if (( sector[0x36] == 'F' ) & ( sector[0x37] == 'A' ) & ( sector[0x38] == 'T' ) )
            {
                /* Well, this is not a MBR, but a FAT12/16 header!*/
                pstart = 0;
                pactive = 0x80;
                ptype = 0x06;
                psize = 0xFFFFFFFF;
            }
            else if (( sector[0x52] == 'F' ) & ( sector[0x53] == 'A' ) & ( sector[0x54] == 'T' ) )
            {
                /*  Well, this is not a MBR, but a FAT32 header!*/
                pstart = 0;
                pactive = 0x80;
                ptype = 0x06;
                psize = 0xFFFFFFFF;
            }
            else
            {
                pstart = DFS_GetPtnStart( 0, sector, 0, &pactive, &ptype, &psize );
                if ( pstart == 0xffffffff )
                {
                    /*  Cannot find first partition*/
                    return -1;
                }
            }
            return pstart; /* starting sector of MBR of current partition*/
        }
    }
    default:
        break;
    }
}
