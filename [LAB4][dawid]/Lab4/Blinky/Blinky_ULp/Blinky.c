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
#include "Serial.h"


                                                                         
char text[10];
int gsm;

/*----------------------------------------------------------------------------
  Main Program
 *----------------------------------------------------------------------------*/
int main (void) {
char temp;

  SER_Init0();                                /* UART0 Initialization           */
  SER_Init1();                                /* UART1 Initialization           */
  
  SER_PutChar1('s'); 
  SER_PutChar1('t'); 
  SER_PutChar1('a'); 
  SER_PutChar1('r'); 
  SER_PutChar1('t'); 


  while (1) {                              
     
	if (CharAv0()) 
		SER_PutChar1(SER_GetChar0());
	if (CharAv1()){ 
		SER_PutChar0(SER_GetChar1());	
	}


	   					 

    }

   
  
}
