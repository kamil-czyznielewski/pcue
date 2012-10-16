/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     toolbar_spe.c
* @brief    Hardware specific  various utilities for touchscreen toolbar
* @author   YRT
* @date     09/2009
*
* @note     Split from tooolbar.c
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

//-- System toolbar shown by default
const u16 ButtonMenuBmp [] =
{
#include "bmp\Config_RLE.h"
};

const u16 ButtonLowSoundHPBmp [] =
{
#include "bmp\HP-_RLE.h"
};

const u16 ButtonHighSoundHPBmp [] =
{
#include "bmp\HP+_RLE.h"
};

const u16 ButtonMuteHPBmp [] =
{
#include "bmp\HP_off_RLE.h"
};

const u16 ButtonSoundHPBmp [] =
{
#include "bmp\HP_RLE.h"
};

const u16 ButtonLowSoundLSBmp [] =
{
#include "bmp\LS-_RLE.h"
};

const u16 ButtonHighSoundLSBmp [] =
{
#include "bmp\LS+_RLE.h"
};

const u16 ButtonMuteLSBmp [] =
{
#include "bmp\LS_off_RLE.h"
};

const u16 ButtonSoundLSBmp [] =
{
#include "bmp\LS_RLE.h"
};
