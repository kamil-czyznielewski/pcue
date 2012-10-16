#ifndef _UART1_
#define _UART1_

// ****************************************************************************************************

#include <stdarg.h>

// ****************************************************************************************************

extern void UART1_initialize(int rate);
extern void UART1_disable(void);
extern void UART1_waitForTransferCompletion(void);
extern void UART1_putc(char c);
extern void UART1_puts(char *c);
extern void UART1_put32(unsigned int i);
extern void UART1_put16(unsigned short i);
extern void UART1_put8(unsigned char i);
extern char UART1_checkc(void);
extern char UART1_getc(void);
extern int UART1_gets(char *str, int max);
extern void UART1_write(char *c, int size);

// ****************************************************************************************************

#endif
