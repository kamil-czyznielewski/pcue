/*----------------------------------------------------------------------------
 * Name:    Serial.c
 * Purpose: Low Level Serial Routines
 * Note(s): possible defines select the used communication interface:
 *            __DBG_ITM   - ITM SWO interface
 *            __UART0     - COM0 (UART0) interface
 *                        - COM1 (UART1) interface  (default)
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

#include "LPC17xx.H"                         /* LPC17xx definitions           */
#include "Serial.h"

#ifdef __DBG_ITM
volatile int ITM_RxBuffer = ITM_RXBUFFER_EMPTY;  /*  CMSIS Debug Input        */
#endif

#ifdef __UART0
  #define UART    LPC_UART0
#else
  #define UART    LPC_UART1
#endif


/*----------------------------------------------------------------------------
  Initialize UART pins, Baudrate
 *----------------------------------------------------------------------------*/
void SER_Init (void) {
#ifndef __DBG_ITM

#ifdef __UART0                             /* UART0 */
  LPC_SC->PCONP |= ((1 << 3) | (1 << 15)); /* enable power to UART0 & IOCON   */

  LPC_PINCON->PINSEL0 |=  (1 << 4);        /* Pin P0.2 used as TXD0           */
  LPC_PINCON->PINSEL0 |=  (1 << 6);        /* Pin P0.3 used as RXD0           */
#else                                      /* UART1 */
  LPC_SC->PCONP |= ((1 << 4) | (1 << 15)); /* enable power to UART1 & IOCON   */

  LPC_PINCON->PINSEL4 |=  (2 << 0);        /* Pin P2.0 used as TXD1           */
  LPC_PINCON->PINSEL4 |=  (2 << 2);        /* Pin P2.1 used as RXD1           */
#endif

  UART->LCR    = 0x83;                     /* 8 bits, no Parity, 1 Stop bit   */
  UART->DLL    = 9;                        /* 115200 Baud Rate @ 25.0 MHZ PCLK*/
  UART->FDR    = 0x21;                     /* FR 1,507, DIVADDVAL=1, MULVAL=2 */
  UART->DLM    = 0;                        /* High divisor latch = 0          */
  UART->LCR    = 0x03;                     /* DLAB = 0                        */

#endif
}

void SER_Init0 (void) {
#ifndef __DBG_ITM

  LPC_SC->PCONP |= ((1 << 3) | (1 << 15)); /* enable power to UART0 & IOCON   */

  LPC_PINCON->PINSEL0 |=  (1 << 4);        /* Pin P0.2 used as TXD0           */
  LPC_PINCON->PINSEL0 |=  (1 << 6);        /* Pin P0.3 used as RXD0           */

  LPC_UART0->LCR    = 0x83;                     /* 8 bits, no Parity, 1 Stop bit   */
  LPC_UART0->DLL    = 9;                        /* 115200 Baud Rate @ 25.0 MHZ PCLK*/
  LPC_UART0->FDR    = 0x21;                     /* FR 1,507, DIVADDVAL=1, MULVAL=2 */
  LPC_UART0->DLM    = 0;                        /* High divisor latch = 0          */
  LPC_UART0->LCR    = 0x03;                     /* DLAB = 0                        */

#endif
}

void SER_Init1 (void) {
#ifndef __DBG_ITM

  LPC_SC->PCONP |= ((1 << 4) | (1 << 15)); /* enable power to UART1 & IOCON   */

  LPC_PINCON->PINSEL4 |=  (2 << 0);        /* Pin P2.0 used as TXD1           */
  LPC_PINCON->PINSEL4 |=  (2 << 2);        /* Pin P2.1 used as RXD1           */

  LPC_UART1->LCR    = 0x83;                     /* 8 bits, no Parity, 1 Stop bit   */
  LPC_UART1->DLL    = 9;                        /* 115200 Baud Rate @ 25.0 MHZ PCLK*/
  LPC_UART1->FDR    = 0x21;                     /* FR 1,507, DIVADDVAL=1, MULVAL=2 */
  LPC_UART1->DLM    = 0;                        /* High divisor latch = 0          */
  LPC_UART1->LCR    = 0x03;                     /* DLAB = 0                        */

#endif
}


/*----------------------------------------------------------------------------
  Write character to Serial Port
 *----------------------------------------------------------------------------*/
int SER_PutChar (int c) {

#ifdef __DBG_ITM
    ITM_SendChar(c);
#else
  while (!(UART->LSR & 0x20));
  UART->THR = c;
#endif
  return (c);
}
int SER_PutChar0 (int c) {

#ifdef __DBG_ITM
    ITM_SendChar(c);
#else
  while (!(LPC_UART0->LSR & 0x20));
  LPC_UART0->THR = c;
#endif
  return (c);
}
int SER_PutChar1 (int c) {

#ifdef __DBG_ITM
    ITM_SendChar(c);
#else
  while (!(LPC_UART1->LSR & 0x20));
  LPC_UART1->THR = c;
#endif
  return (c);
}


/*----------------------------------------------------------------------------
  Read character from Serial Port   (blocking read)
 *----------------------------------------------------------------------------*/
int SER_GetChar (void) {

#ifdef __DBG_ITM
  while (ITM_CheckChar() != 1) __NOP();
  return (ITM_ReceiveChar());
#else
  while (!(UART->LSR & 0x01));
  return (UART->RBR);
#endif
}
int SER_GetChar0 (void) {

#ifdef __DBG_ITM
  while (ITM_CheckChar() != 1) __NOP();
  return (ITM_ReceiveChar());
#else
  while (!(LPC_UART0->LSR & 0x01));
  return (LPC_UART0->RBR);
#endif
}
int SER_GetChar1 (void) {

#ifdef __DBG_ITM
  while (ITM_CheckChar() != 1) __NOP();
  return (ITM_ReceiveChar());
#else
  while (!(LPC_UART1->LSR & 0x01));
  return (LPC_UART1->RBR);
#endif
}

int jestZnak0(){
	return (LPC_UART0->LSR & 0x01);
}
int jestZnak1(){
	return (LPC_UART1->LSR & 0x01);
}
void wyslij0(char *dane){
	while(*dane){
		SER_PutChar0(*dane);
		++dane;
	}
	SER_PutChar0((char)10);
	SER_PutChar0((char)13);
}
void wyslij1(char *dane){
	while(*dane){
		SER_PutChar1(*dane);
		++dane;
	}
	SER_PutChar1((char)10);
	SER_PutChar1((char)13);
}
void odbierz0(char *dane){
	while(1){
		*dane=SER_GetChar0();
		++dane;
		if((*(dane-1)=='K')&&(*(dane-2)=='O')){
			*dane=0;
			return;
		}
	}
}
