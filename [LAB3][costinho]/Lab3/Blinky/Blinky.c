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
#include <math.h>

#define __FI        1                       /* Font index 16x24               */
                                                                         
char text[10];
int methodLed = 1;
char *index = "8976x";

/* Import external variables from IRQ.c file                                  */
extern uint8_t  clock_1s;

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

  if(methodLed==1) methodLed=2;
  else methodLed=1;
}

/*----------------------------------------------------------------------------
  Main Program
 *----------------------------------------------------------------------------*/
int main (void) {
  uint32_t ad_avg = 0;
  uint16_t ad_val = 0, ad_val_ = 0xFFFF;
  uint32_t ad_procent = 0;
  uint32_t ad_led_status = 0;
  int i; 

  BUTTON_Init();
  LED_Init();                                /* LED Initialization            */
  SER_Init();                                /* UART Initialization           */
  ADC_Init();                                /* ADC Initialization            */

#ifdef __USE_LCD
  GLCD_Init();                               /* Initialize graphical LCD      */

  GLCD_Clear(White);                         /* Clear graphical LCD display   */
  GLCD_SetBackColor(Blue);
  GLCD_SetTextColor(White);
  GLCD_DisplayString(0, 0, __FI, "    Application    ");
  GLCD_DisplayString(1, 0, __FI, "     by student     ");
  GLCD_DisplayString(2, 0, __FI, "    PUT  Poznan    ");
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

      ad_procent = ceil(((double)(ad_val * 100)) / 4060.0);
      if(ad_procent > 100) ad_procent = 100;
      sprintf(text, "%d%%  ", ad_procent);
#ifdef __USE_LCD
      GLCD_SetTextColor(Red);
      GLCD_DisplayString(5,  9, __FI,  (unsigned char *)text);
      GLCD_Bargraph (144, 6*24, 176, 20, (ad_val >> 2)); /* max bargraph is 10 bit */
#endif
    }

    ad_led_status = floor(((float)ad_procent)/12.5f);
    if(ad_led_status==8)ad_led_status=7;
	
    LED_On(ad_led_status);
    if(methodLed == 1) {
	  	for(i = 0; i < 8; i++){
	    	if (i != ad_led_status) { LED_Off(i); }
	  	}
	  }
    else if(methodLed == 2) {
		 for(i = 0; i < 8; i++){
			if(i <= ad_led_status) {
			 	LED_On(i);
			}
	    	else LED_Off(i);
	  	}
	  }


    /* Print message with AD value every second                               */
    if (clock_1s) {
      clock_1s = 0;
      printf("AD value: %s\r\n", text);
    }
  }
}
