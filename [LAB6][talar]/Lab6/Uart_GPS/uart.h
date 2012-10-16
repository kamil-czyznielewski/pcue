#ifndef _UART_
#define _UART_

// ****************************************************************************************************

#include <stdarg.h>

// ****************************************************************************************************

extern void UART_initialize(int rate);
extern void UART_disable(void);
extern void UART_waitForTransferCompletion(void);
extern void UART_putc(char c);
extern void UART_puts(char *c);
extern void UART_put32(unsigned int i);
extern void UART_put16(unsigned short i);
extern void UART_put8(unsigned char i);
extern char UART_getc(void);
extern int UART_gets(char *str, int max);
extern void UART_write(char *c, int size);

// ****************************************************************************************************

#endif
