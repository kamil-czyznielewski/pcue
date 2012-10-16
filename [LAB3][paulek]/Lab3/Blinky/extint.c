/*****************************************************************************
 *   extint.c:  External interrupt API C file for NXP LPC17xx 
 *   Family Microprocessors
 *
 *   Copyright(C) 2009, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2009.05.26  ver 1.00    Prelimnary version, first Release
 *
*****************************************************************************/
#include <lpc17xx.h>
#include "extint.h"
#include "type.h"
volatile uint32_t eint0_counter;

/*****************************************************************************
** Function name:		EINT0_Handler
**
** Descriptions:		external INT handler
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void EINT0_IRQHandler (void) 
{
  SC -> EXTINT = EINT0;		/* clear interrupt */
		
  eint0_counter++;
  if ( eint0_counter & 0x01 )	/* alternate the LED display */
  {
	typ = 0;
  }
  else
  {
	typ = 1;	/* turn off P2.4~7 */
  }

}

/*****************************************************************************
** Function name:		EINTInit
**
** Descriptions:		Initialize external interrupt pin and
**						install interrupt handler
**
** parameters:			None
** Returned value:		true or false, return false if the interrupt
**						handler can't be installed to the VIC table.
** 
*****************************************************************************/
uint32_t EINTInit( void )
{

  PINCON -> PINSEL4 = 0x00100000;	/* set P2.10 as EINT0 and
							P2.0~7 GPIO output */
  GPIO2 -> FIODIR = 0x000000FF;	/* port 2, bit 0~7 only */
  GPIO2 -> FIOCLR = 0x000000FF;	/* turn off LEDs */

  GPIOINT -> IO2IntEnF = 0x200;	/* Port2.10 is falling edge. */
  SC -> EXTMODE = EINT0_EDGE;	/* INT0 edge trigger */
  SC -> EXTPOLAR = 0;			/* INT0 is falling edge by default */

	NVIC_EnableIRQ(EINT0_IRQn);

  return( TRUE );
}

/******************************************************************************
**                            End Of File
******************************************************************************/

