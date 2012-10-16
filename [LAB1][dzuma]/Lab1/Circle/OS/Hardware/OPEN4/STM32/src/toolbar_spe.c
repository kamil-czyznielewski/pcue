/********************* (C) COPYRIGHT 2009 RAISONANCE S.A.S. *******************/
/**
*
* @file     toolbar_spe.c
* @brief    Hardware specific  various utilities for touchscreen toolbar
* @author   YRT
* @date     09/2009
*
* @note     Open4 STM32
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
#include "bmp\Config_60RLE.h"
//    #include "bmp\Config.bmp.h"
};

const u16 ButtonLowSoundHPBmp [] =
{
#include "bmp\HP-_60RLE.h"
//    #include "bmp\HP-.bmp.h"
};

const u16 ButtonHighSoundHPBmp [] =
{
#include "bmp\HP+_60RLE.h"
//    #include "bmp\HP+.bmp.h"
};

const u16 ButtonMuteHPBmp [] =
{
#include "bmp\HP_off_60RLE.h"
//    #include "bmp\HP_off.bmp.h"
};

const u16 ButtonSoundHPBmp [] =
{
#include "bmp\HP_60RLE.h"
//    #include "bmp\HP.bmp.h"
};

const u16 ButtonLowSoundLSBmp [] =
{
#include "bmp\LS-_60RLE.h"
//    #include "bmp\LS-.bmp.h"
};

const u16 ButtonHighSoundLSBmp [] =
{
#include "bmp\LS+_60RLE.h"
//    #include "bmp\LS+.bmp.h"
};

const u16 ButtonMuteLSBmp [] =
{
#include "bmp\LS_off_60RLE.h"
//    #include "bmp\LS_off.bmp.h"
};

const u16 ButtonSoundLSBmp [] =
{
#include "bmp\LS_60RLE.h"
//    #include "bmp\LS.bmp.h"
};

