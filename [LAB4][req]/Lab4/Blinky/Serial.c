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

//J: to nas nie bêdzie interesowaæ
//#ifdef __UART0
//  #define UART    LPC_UART0
//#else
//  #define UART    LPC_UART1
//#endif


/*----------------------------------------------------------------------------
  Initialize UART pins, Baudrate
 *----------------------------------------------------------------------------*/
void SER_Init (void) {
#ifndef __DBG_ITM

//#ifdef __UART0                             /* UART0 */
  LPC_SC->PCONP |= ((1 << 3) | (1 << 15)); /* enable power to UART0 & IOCON   */

  LPC_PINCON->PINSEL0 |=  (1 << 4);        /* Pin P0.2 used as TXD0           */
  LPC_PINCON->PINSEL0 |=  (1 << 6);        /* Pin P0.3 used as RXD0           */
//#else                                      /* UART1 */
  LPC_SC->PCONP |= ((1 << 4) | (1 << 15)); /* enable power to UART1 & IOCON   */

  LPC_PINCON->PINSEL4 |=  (2 << 0);        /* Pin P2.0 used as TXD1           */
  LPC_PINCON->PINSEL4 |=  (2 << 2);        /* Pin P2.1 used as RXD1           */
//#endif

  //J: to samo musimy powtórzyæ dla UART1
  LPC_UART0->LCR    = 0x83;                /* 8 bits, no Parity, 1 Stop bit   */
  LPC_UART0->DLL    = 9;                   /* 115200 Baud Rate @ 25.0 MHZ PCLK*/
  LPC_UART0->FDR    = 0x21;                /* FR 1,507, DIVADDVAL=1, MULVAL=2 */
  LPC_UART0->DLM    = 0;                   /* High divisor latch = 0          */
  LPC_UART0->LCR    = 0x03;                /* DLAB = 0                        */

  //J: to powtarzamy
  LPC_UART1->LCR    = 0x83;                /* 8 bits, no Parity, 1 Stop bit   */
  LPC_UART1->DLL    = 9;                   /* 115200 Baud Rate @ 25.0 MHZ PCLK*/
  LPC_UART1->FDR    = 0x21;                /* FR 1,507, DIVADDVAL=1, MULVAL=2 */
  LPC_UART1->DLM    = 0;                   /* High divisor latch = 0          */
  LPC_UART1->LCR    = 0x03;                /* DLAB = 0                        */
#endif
}


/*----------------------------------------------------------------------------
  Write character to Serial Port 0
 *----------------------------------------------------------------------------*/
//J: wersja dla portu UART0
int SER0_PutChar (int c) {

#ifdef __DBG_ITM
    ITM_SendChar(c);
#else
  while (!(LPC_UART0->LSR & 0x20));
  LPC_UART0->THR = c;
#endif
  return (c);
}


/*----------------------------------------------------------------------------
  Write character to Serial Port 1
 *----------------------------------------------------------------------------*/
//J: wersja dla portu UART1
int SER1_PutChar (int c) {

#ifdef __DBG_ITM
    ITM_SendChar(c);
#else
  while (!(LPC_UART1->LSR & 0x20));
  LPC_UART1->THR = c;
#endif
  return (c);
}


/*----------------------------------------------------------------------------
  Read character from Serial Port 0  (non-blocking read)
 *----------------------------------------------------------------------------*/
//J: przerobienie wersji blokuj¹cej na nie blokuj¹ca
int SER0_GetChar (void) {

#ifdef __DBG_ITM
  while (ITM_CheckChar() != 1) __NOP();
  return (ITM_ReceiveChar());
#else
  if (!(LPC_UART0->LSR & 0x01)) //sprawdŸ czy flaga oczekuj¹cego znaku jest ustawiona
  {
    return 0; //zwróæ 0, gdy nie ma ¿adnego oczekuj¹cego znaku
  } else {
    return (LPC_UART0->RBR); //zwróæ znak, gdy jakiœ zosta³ ostatnio odebrany
  }
#endif
}

/*----------------------------------------------------------------------------
  Read character from Serial Port 1  (non-blocking read)
 *----------------------------------------------------------------------------*/
//J: wersja dla UART1
int SER1_GetChar (void) {

#ifdef __DBG_ITM
  while (ITM_CheckChar() != 1) __NOP();
  return (ITM_ReceiveChar());
#else
  if (!(LPC_UART1->LSR & 0x01)) //sprawdŸ czy flaga oczekuj¹cego znaku jest ustawiona
  {
    return 0; //zwróæ 0, gdy nie ma ¿adnego oczekuj¹cego znaku
  } else {
    return (LPC_UART1->RBR); //zwróæ znak, gdy jakiœ zosta³ ostatnio odebrany
  }
#endif
}


/*----------------------------------------------------------------------------
  Write C string to Serial Port 0
 *----------------------------------------------------------------------------*/
void SER0_PutStr (char *c) {
  int i;
  for (i=0; c[i]!=0; i++)
  {
    SER0_PutChar(c[i]);
  }
}


/*----------------------------------------------------------------------------
  Write C string to Serial Port 1
 *----------------------------------------------------------------------------*/
void SER1_PutStr (char *c) {
  int i;
  for (i=0; c[i]!=0; i++)
  {
    SER1_PutChar(c[i]);
  }
}
