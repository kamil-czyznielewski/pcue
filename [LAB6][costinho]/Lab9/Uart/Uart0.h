#ifndef _UART0_
#define _UART0_

// ****************************************************************************************************

#include <stdarg.h>

// ****************************************************************************************************

extern void UART0_initialize(int rate);
extern void UART0_disable(void);
extern void UART0_waitForTransferCompletion(void);
extern void UART0_putc(char c);
extern void UART0_puts(char *c);
extern void UART0_put32(unsigned int i);
extern void UART0_put16(unsigned short i);
extern void UART0_put8(unsigned char i);
extern char UART0_checkc(void);
extern char UART0_getc(void);
extern int UART0_gets(char *str, int max);
extern void UART0_write(char *c, int size);

// ****************************************************************************************************

#endif
