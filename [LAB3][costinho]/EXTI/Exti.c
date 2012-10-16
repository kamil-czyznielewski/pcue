/*----------------------------------------------------------------------------
 * Name:    Exti.c
 * Purpose: Demonstrate use of external interrupt
 * Note(s): 
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2008-2011 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include "LPC17xx.H"                       /* LPC17xx definitions             */
#include "LED.h"
                  
int idxCur = -1;                           /* current led position from 0..7  */
int idxOld =  0;                           /* old     led position from 0..7  */
int dir    =  1;                           /* direction for switching the LED */

/*----------------------------------------------------------------------------
  Function that initializes Button INT0 to generate an interrupt
 *----------------------------------------------------------------------------*/
void BUTTON_Init(void) {

  LPC_GPIO2->FIODIR      &= ~(1 << 10);    /* PORT2.10 defined as input       */
  LPC_GPIOINT->IO2IntEnF |=  (1 << 10);    /* enable falling edge irq         */

  NVIC_EnableIRQ(EINT3_IRQn);              /* enable irq in nvic              */
}


/*----------------------------------------------------------------------------
  External IRQ Handler
 *----------------------------------------------------------------------------*/
void EINT3_IRQHandler()
{

  LPC_GPIOINT->IO2IntClr |= (1 << 10);     /* clear pending interrupt         */

  /* Calculate 'idx': 0,1,...,LED_NUM-1,LED_NUM-1,...,1,0,0,...   */
  idxCur += dir;
  if (idxCur == LED_NUM) { dir = -1; idxCur -=  2; } 
  else if   (idxCur < 0) { dir =  1; idxCur +=  2; }

  LED_Off(idxOld);                         /* switch off    old LED position  */
  LED_On (idxCur);                         /* switch on  current LED position */
  idxOld = idxCur;
}


/*----------------------------------------------------------------------------
  Main Program
 *----------------------------------------------------------------------------*/
int main (void) {

  LED_Init();
  BUTTON_Init();

  idxCur = 0;                              /* current led position from 0..7  */
  idxOld = 0;                              /* old     led position from 0..7  */
  dir    = 1;                              /* direction for switching the LED */
  LED_On (idxCur);                         /* switch on  first LED            */


  while (1);
}
