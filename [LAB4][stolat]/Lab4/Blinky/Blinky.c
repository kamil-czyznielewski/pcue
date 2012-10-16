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

/* Import external variables from IRQ.c file                                  */
extern uint8_t  clock_1s;


/*----------------------------------------------------------------------------
  Main Program
 *----------------------------------------------------------------------------*/
int main (void) {

  uint32_t ad_avg = 0;
  uint16_t ad_val = 0, ad_val_ = 0xFFFF;

	//
	char z,dane[100];
	int i=0;
	SER_Init0();
	SER_Init1();
	wyslij0("at+cops?");
	odbierz0(dane);
	i=0;
	while(dane[i]!=':')++i;
	++i;
	while((dane[i]<'0')||(dane[i]>'9')){
		++i;
	}
	switch(dane[i]){
		case '0':wyslij1("choose network automatically");break;
		case '1':wyslij1("choose network manually");break;
		//case '3':wyslij1("choose network automatically");break;
		//case '4':wyslij1("choose network automatically");break;
		default:wyslij1(dane+i);break;
	}
	wyslij0("at+creg?");
	odbierz0(dane);
	i=0;
	while(dane[i]!=':')++i;
	++i;
	while((dane[i]<'0')||(dane[i]>'9')){
		++i;
	}
	/*switch(dane[i]){
		case '0':wyslij1("Forbid network registration to provide result code(default setting)");break;
		case '1':wyslij1("allow network registration to provide result code");break;
		//case '3':wyslij1("choose network automatically");break;
		//case '4':wyslij1("choose network automatically");break;
		default:wyslij1(dane+i);break;
	}*/
	++i;
	while((dane[i]<'0')||(dane[i]>'9')){
		++i;
	}
	switch(dane[i]){
		case '0':wyslij1("Unregistered, terminal isn’t searching for new operator");break;
		case '1':wyslij1("Registered to local network");break;
		//case '3':wyslij1("choose network automatically");break;
		//case '4':wyslij1("choose network automatically");break;
		default:wyslij1(dane+i);break;
	}
	wyslij0("at*tsimins?");
	odbierz0(dane);
	i=0;
	while(dane[i]!=':')++i;
	++i;
	while((dane[i]<'0')||(dane[i]>'9')){
		++i;
	}
	++i;
	while((dane[i]<'0')||(dane[i]>'9')){
		++i;
	}
	switch(dane[i]){
		case '0':wyslij1("no SIM card");break;
		case '1':wyslij1("SIM card");break;
		//case '3':wyslij1("choose network automatically");break;
		//case '4':wyslij1("choose network automatically");break;
		default:wyslij1(dane+i);break;
	}
	wyslij0("at+cmgf=1");
	odbierz0(dane);
	wyslij1("wlaczenie trybu tekstowego");
	wyslij0("at+cmgr=1");
	//odbierz0(dane);
	while(1){
		if(jestZnak0()){
			z=SER_GetChar0();
			SER_PutChar1(z);
		}
		if(jestZnak1()){
			z=SER_GetChar1();
			SER_PutChar0(z);
		}
	}
	//

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

      sprintf(text, "0x%04X", ad_val);       /* format text for print out     */
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
