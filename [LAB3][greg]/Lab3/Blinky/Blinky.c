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
         
  
/*

Uwaga!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Zakomentowano także jedną metodę w pliku IRQ.c  - nie wiem czy tak trzeba - ale bez tego mi diody
mrygały zamiast robić to co ja chciałem.

*/       
                                                                         
char text[10];
/*
mode == 1 - tylko jedna dioda zaświecona
mode == 2 - wszystkie diody od początku do konretnej
*/
int mode = 1;

/* Import external variables from IRQ.c file                                  */
extern uint8_t  clock_1s;

//obsługa przerwania
void EINT3_IRQHandler()
{
	//potwierdzenie obsługi przerwania - jak tego nie będzie to program wisi
	LPC_GPIOINT->IO2IntClr |= (1 << 10);
	
	//zmiana moda :-)
	if(mode == 1) mode = 2;
	else mode = 1;
}

//init przyciski - nie mam pojęcia o co chodzi :-)
void BUTTON_Init(void) {

  LPC_GPIO2->FIODIR      &= ~(1 << 10);    /* PORT2.10 defined as input       */
  LPC_GPIOINT->IO2IntEnF |=  (1 << 10);    /* enable falling edge irq         */

  NVIC_EnableIRQ(EINT3_IRQn);              /* enable irq in nvic              */
}

/*----------------------------------------------------------------------------
  Main Program
 *----------------------------------------------------------------------------*/
int main (void) {
  uint32_t ad_avg = 0;
  uint16_t ad_val = 0, ad_val_ = 0xFFFF;
  
  /*
  	percent - liczba określająca procent
  	led_num - numer diody odpowiedniej dla danego procenta
  	tmp - jakaś zmienna używana w for'ze do iteracji..
  */
  int tmp;
  int percent;
  float led_num;
  
  BUTTON_Init();
  LED_Init();                                /* LED Initialization            */
  SER_Init();                                /* UART Initialization           */
  ADC_Init();                                /* ADC Initialization            */

#ifdef __USE_LCD
  GLCD_Init();                               /* Initialize graphical LCD      */

  GLCD_Clear(White);                         /* Clear graphical LCD display   */
  GLCD_SetBackColor(Blue);
  GLCD_SetTextColor(White);
  GLCD_DisplayString(0, 0, __FI, "    MCB1700 Demo    ");
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
      
      /*
      	sprintf jest zmieniony - 4079 maksymalna wartość - zadanie polegało na wyświetleniu 
      	w procentach zamiast hex'ach - więc konwertujemy
      */	  
      sprintf(text, "%3d", (ad_val *100)/4079);       /* format text for print out     */
	  //pobieramy liczbę do percenta
	  percent = atoi(text);
	  //mamy 8 ledow 100/8 = 12.5 i obliczamy która odpowiada naszej wartosci
	  led_num = percent/12.5f;

		//mode 1 - świeci sę tylko jedna dioda więc inne zgaszamy
	  if(mode == 1) {
	  	for(tmp = 0; tmp < 8; tmp++){
	    	LED_Off(tmp);
	  	}
	  	// mode 2 - świecą się także te na lewo od właściwej więc je zapalamy a te na prawo zgaszamy
	  } else if(mode == 2) {
		for(tmp = 0; tmp < 8; tmp++){
			if(tmp < led_num) {
			 	LED_On(tmp);
			}
	    	else LED_Off(tmp);
	  	}
	  }
	  //zapalamy odpowidnia diode
	  LED_On(led_num);
	  
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
    }
  }
}
