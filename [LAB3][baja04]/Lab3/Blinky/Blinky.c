/*----------------------------------------------------------------------------
 * Name:    Blinky.c
 * Purpose: LED Flasher
 * Note(s): possible defines set in "options for target - C/C++ - Define"
 *            __USE_LCD   - enable Output on LCD
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

#include <stdio.h>
#include "LPC17xx.H"                         /* LPC17xx definitions           */
#include "GLCD.h"
#include "Serial.h"
#include "LED.h"
#include "ADC.h"

#define __FI        1                       /* Font index 16x24               */
                                                                         
char text[10];
char tytul[10];
unsigned int adpercentagevalue = 0;

/* Import external variables from IRQ.c file                                  */
extern uint8_t  clock_1s;
extern unsigned int ile;
int typ;
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
  if(typ == 0 ) 
   typ = 1;
   else typ = 0;
}


/*----------------------------------------------------------------------------
  Main Program
 *----------------------------------------------------------------------------*/
int main (void) {
  
  uint32_t ad_avg = 0;
  uint16_t ad_val = 0, ad_val_ = 0xFFFF;
  sprintf(tytul,"       %s       ","Blazej");/* Text						  */
  LED_Init();                                /* LED Initialization            */
  SER_Init();                                /* UART Initialization           */
  ADC_Init();
  BUTTON_Init();
#ifdef __USE_LCD
  GLCD_Init();                               /* Initialize graphical LCD      */
  GLCD_Clear(White);                         /* Clear graphical LCD display   */
  GLCD_SetBackColor(Blue);
  GLCD_SetTextColor(White);
  GLCD_DisplayString(0, 0, __FI,(unsigned char *)tytul);
  GLCD_DisplayString(1, 0, __FI, "       Blinky       ");
  GLCD_DisplayString(2, 0, __FI, "    www.keil.com    ");
  GLCD_SetBackColor(White);
  GLCD_SetTextColor(Blue);
  GLCD_DisplayString(5, 0, __FI, "AD value:            ");
#endif

  SysTick_Config(SystemCoreClock/100);       /* Generate interrupt each 10 ms */
  while (1) {                                /* Loop forever                  */
    /* AD converter input                                                     */
    if (AD_done) {                           /* If conversion has finished    */
      AD_done = 0;

      ad_avg += AD_last << 8;                /* Add AD value to averaging     */
      ad_avg ++;
      if ((ad_avg & 0xFF) == 0x10) {         /* average over 16 values        */
        ad_val = (ad_avg >> 8) >> 4;         /* average devided by 16         */
        ad_avg = 0;
      }
    }

    if (ad_val ^ ad_val_) {                  /* AD value changed              */
      ad_val_ = ad_val;

      sprintf(text, "    %3d%%", (int)((ad_val*100)/4070));       /* format text for print out     */
	  
	  adpercentagevalue = (unsigned int)((ad_val*100)/4070);
#ifdef __USE_LCD
      GLCD_SetTextColor(Red);
      GLCD_DisplayString(5,  9, __FI,  (unsigned char *)text);
      GLCD_Bargraph (144, 6*24, 176, 20, (ad_val >> 2)); /* max bargraph is 10 bit */
#endif
    }

    /* Print message with AD value every second                               */
    if (clock_1s) {
      clock_1s = 0;
      printf("AD value: %s\r\n", text);
	  ile = adpercentagevalue;
	  LED_Off(0);
	  LED_Off(1);
	  LED_Off(2);
	  LED_Off(3);
	  LED_Off(4);
	  LED_Off(5);
	  LED_Off(6);
	  LED_Off(7);
	  if(typ == 0 ){
	  if(ile >= 0 && ile < 12){
	  	LED_On(0);
	  }
	  if(ile >= 12 && ile < 25){
	  	LED_On(0);
	  	LED_On(1);
	  }
	  if(ile >= 25 && ile < 37){
	  	LED_On(0);
	  	LED_On(1);
		LED_On(2);
	  }
	  if(ile >= 37 && ile < 49){
	  	LED_On(0);
	  	LED_On(1);
		LED_On(2);
		LED_On(3);
	  }
	  if(ile >= 49 && ile < 61){
	  	LED_On(0);
	  	LED_On(1);
		LED_On(2);
		LED_On(3);
		LED_On(4);
	  }
	  if(ile >= 61 && ile < 74){
	  	LED_On(0);
	  	LED_On(1);
		LED_On(2);
		LED_On(3);
		LED_On(4);
		LED_On(5);
	  }
	  if(ile >= 74 && ile < 86){
	  	LED_On(0);
	  	LED_On(1);
		LED_On(2);
		LED_On(3);
		LED_On(4);
		LED_On(5);
		LED_On(6);
	  }
	  if(ile >= 86 && ile <= 100){
	  	LED_On(0);
	  	LED_On(1);
		LED_On(2);
		LED_On(3);
		LED_On(4);
		LED_On(5);
		LED_On(6);
		LED_On(7);
	  }
	  }
	  else {
	   if(ile >= 0 && ile < 12){
	  	LED_On(0);
	  }
	  if(ile >= 12 && ile < 25){
	  	LED_On(1);
	  }
	  if(ile >= 25 && ile < 37){

		LED_On(2);
	  }
	  if(ile >= 37 && ile < 49){

		LED_On(3);
	  }
	  if(ile >= 49 && ile < 61){

		LED_On(4);
	  }
	  if(ile >= 61 && ile < 74){

		LED_On(5);
	  }
	  if(ile >= 74 && ile < 86){

		LED_On(6);
	  }
	  if(ile >= 86 && ile <= 100){

		LED_On(7);
	  }
	  	
	  }	 
    }
  }
}
