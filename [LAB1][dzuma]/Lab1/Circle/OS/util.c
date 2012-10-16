/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     Util.c
* @brief    Various common utilities for CircleOS.
* @author   RT
* @date     07/2007
* @version  4.0 Add Open4 Primer
* @date     10/2009 
*
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private defines -----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
enum eSpeed CurrentSpeed;

/* Private variables ---------------------------------------------------------*/
bool fTemperatureInFahrenheit = 0;  /*!< 1 : Fahrenheit, 0 : Celcius (default). */

/* Private function prototypes -----------------------------------------------*/
static void _int2str( u8* ptr, int_t X, len_t digit, bool flagunsigned, bool fillwithzero );

/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
*
*                                vbattoa
*
*******************************************************************************/
/**
*
*  This function convert an u16 in ascii radix 10
*
*  @param[out] ptr   A pointer to a string where the converted value will be put.
*  @param[in]  X     The value to convert.
*
*  @see  DRAW_DisplayVbat
*
**/
/******************************************************************************/
NODEBUG2 void vbattoa( u8* ptr, u16 X )
{
    u8 c;
    u16 r = 0;

    /* 1 000 digit*/
    c = (( X - r ) / 1000 );
    r = r + ( c * 1000 );
    *ptr++ = c + 0x30;

    /* Dot*/
    *ptr++ = '.';

    /* 100 digit*/
    c = (( X - r ) / 100 );
    r = r + ( c * 100 );
    *ptr++ = c + 0x30;

    /* 10 digit*/
    c = (( X - r ) / 10 );
    r = r + ( c * 10 );
    *ptr++ = c + 0x30;

    /* Volt*/
    *ptr++ = 'V';
    *ptr++ = 0;
}

/*******************************************************************************
*
*                    _int2str
*
*******************************************************************************/
/**
*
*  Translate a integer into a string.
*
*  @param[in,out] ptr            A pointer to a string large enough to contain
*                                the translated 32 bit word.
*  @param[in]     X              The integer to translate.
*  @param[in]     digit          The amount of digits wanted in the result string.
*  @param[in]     flagunsigned   Is the input word unsigned?
*  @param[in]     fillwithzero   Fill with zeros or spaces.
*
**/
/******************************************************************************/
NODEBUG2 static void _int2str( u8* ptr, int_t X, len_t digit, bool flagunsigned, bool fillwithzero )
{
    u8      c;
    bool    fFirst   = 0;
    bool    fNeg     = 0;
    uint_t  DIG      = 1;
    len_t   i;
    int_t   r;

    for ( i = 1; i < digit; i++ )
    {
        DIG *= 10;
    }

    if ( !flagunsigned && ( X < 0 ) )
    {
        fNeg = 1;
        r    = -X;
    }
    else
    {
        r = X;
    }

    for ( i = 0; i < digit; i++, DIG /= 10 )
    {
        c  = ( r / DIG );
        r -= ( c * DIG );

        if ( fillwithzero || fFirst || c || ( i == ( digit - 1 ) ) )
        {
            if (( fFirst == 0 ) && !flagunsigned )
            {
                *ptr++ = fNeg ? '-' : ' ';
            }

            *ptr++ = ( c % 10 ) + '0';
            fFirst = 1;
        }
        else
        {
            *ptr++ = ' ';
        }
    }

    *ptr++ = '\0';
}

/* Public functions for CircleOS ---------------------------------------------*/


/// @endcond

/* Public functions ----------------------------------------------------------*/

/*******************************************************************************
*
*                    UTIL_SetTempMode
*
*******************************************************************************/
/**
*
*  Set the temperature mode (F/C)
*
*  @param[in]     mode       0: Celcius, 1: Fahrenheit
*
**/
/******************************************************************************/
void UTIL_SetTempMode( bool mode )
{
    fTemperatureInFahrenheit = mode;

    return;
}

/*******************************************************************************
*
*                   UTIL_uint2str
*
*******************************************************************************/
/**
*
*  Convert an <b>unsigned</b> integer into a string.
*  Using this function leads to much smaller code than using the sprintf()
*  function from the C library.
*
*  @param [out]  ptr    The output string.
*  @param [in]   X      The unsigned value to convert.
*  @param [in]   digit  The number of digits in the output string.
*  @param [in]   fillwithzero  \li 0   fill with blanks.
*                              \li 1   fill with zeros.
*
**/
/********************************************************************************/
void UTIL_uint2str( u8* ptr, uint_t X, len_t digit, bool fillwithzero )
{
    _int2str( ptr, X, digit, 1, fillwithzero );
}

/*******************************************************************************
*
*                   UTIL_int2str
*
*******************************************************************************/
/**
*
*  Convert a <b>signed</b> integer into a string.
*  Using this function leads to much smaller code than using the sprintf()
*  function from the C library.
*
*  @param [out]  ptr    The output string.
*  @param [in]   X      The unsigned value to convert.
*  @param [in]   digit  The number of digits in the output string.
*  @param [in]   fillwithzero  \li 0   fill with blanks.
*                              \li 1   fill with zeros.
*
**/
/******************************************************************************/
void UTIL_int2str( u8* ptr, int_t X, len_t digit, bool fillwithzero )
{
    _int2str( ptr, X, digit, 0, fillwithzero );
}

/*******************************************************************************
*
*                                UTIL_GetPll
*
*******************************************************************************/
/**
*
*  Get clock frequency
*
*  @return   Current clock speed from very low to very fast.
*
**/
/******************************************************************************/
enum eSpeed UTIL_GetPll( void )
{
    return CurrentSpeed;
}

/*******************************************************************************
*
*                                UTIL_GetVersion
*
*******************************************************************************/
/**
*
*  Get CircleOS version.
*
*  @return  A pointer to a string containing the CircleOS version.
*
**/
/******************************************************************************/
const u8* UTIL_GetVersion( void )
{
    return STR_OSVERSION;
}


/*******************************************************************************
*
*                                UTIL_SetSchHandler
*
*******************************************************************************/
/**
*
*  Redirect a SCHEDULER handler.
*  Set the current SCHEDULER handler. With UTIL_GetSchHandler(), these functions
*  allow to take the control of the different handler. You can:
*        - replace them (get-Set)by your own handler
*        - disable a handler: UTIL_SetSchHandler(Ix,0);
*        - create a new handler (using the unused handlers).
*  See scheduler.c to understand further...
*
*  @param[in]  Ix   ID if the SCH Handler
*  @param[in]  pHDL   Pointer to the handler.
*
**/
/********************************************************************************/
void UTIL_SetSchHandler( enum eSchHandler Ix, tHandler pHDL )
{
    if ( Ix < COUNTOF( SchHandler ) )
        SchHandler[Ix] = pHDL;
}

/*******************************************************************************
*
*                                UTIL_GetSchHandler
*
*******************************************************************************/
/**
*
*  Get the current SCHEDULER handler. With UTIL_SetSchHandler(), these functions
*  allow to take the control of the different handler. You can:
*        - replace them (get-Set)by your own handler
*        - disable a handler: UTIL_SetSchHandler(Ix,0);
*        - create a new handler (using the unused handlers).
*  See scheduler.c to understand further...
*
*  @param[in]  Ix   ID is the SCH Handler
*  @return  A pointer to the current handler, or 0 in case of an error.
*
**/
/********************************************************************************/
tHandler UTIL_GetSchHandler( enum eSchHandler Ix )
{
    if ( Ix < COUNTOF( SchHandler ) )
        return SchHandler[Ix];
    else
        return 0;
}

/*******************************************************************************
*
*                                UTIL_IsStandAloneMode
*
*******************************************************************************/
/**
*
*  Get the current mode of the Primer.
*
*  @return  1 if "stand alone" mode detected.
*           0 if "in base" mode detected.
*
**/
/********************************************************************************/
bool UTIL_IsStandAloneMode(void)
{
    return fIsStandAlone;
}
