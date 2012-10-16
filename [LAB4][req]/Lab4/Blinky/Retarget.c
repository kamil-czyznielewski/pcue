/*----------------------------------------------------------------------------
 * Name:    Retarget.c
 * Purpose: 'Retarget' layer for target-dependent low level functions
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2011 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <rt_misc.h>
#include "Serial.h"

#pragma import(__use_no_semihosting_swi)



struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;


//J: ¿eby wszystkie printfy wypisywa³y tekst na konsolê na porcie UART1
int fputc(int c, FILE *f) {
  return (SER1_PutChar(c));
}

//J: ¿eby wszystkie scanfy pobiera³y dane z portu UART1
int fgetc(FILE *f) {
  return (SER1_GetChar());
}


int ferror(FILE *f) {
  /* Your implementation of ferror */
  return EOF;
}

//J: nawet nie wiem po co to
void _ttywrch(int c) {
  SER1_PutChar(c);
}


void _sys_exit(int return_code) {
label:  goto label;  /* endless loop */
}
