// *****************************************************************************/

#include "ccblkfn.h"
#include "sysreg.h"
#include "Uart.h"

#include <cDEFBF537.h>
#include <sys/exception.h>
#include "ccblkfn.h"
#include "sysreg.h"
#include <stdarg.h>
#include <string.h> 

EX_INTERRUPT_HANDLER(PORTF_IntA_ISR)
{
	*pPORTFIO_CLEAR = 0x0FC4;

	UART_putc(' ');
	UART_putc('x');
	UART_putc('X');
	UART_putc('x');
	UART_putc('X');
	UART_putc('x');
	UART_putc(' ');
}

void Init_Interrupts(void)
{
	// assign core IDs to interrupts
	*pSIC_IAR0 = 0xffffffff;
	*pSIC_IAR1 = 0xffffffff;
	//*pSIC_IAR2 = 0xffff4fff;					// Timer0 -> ID4; 
	*pSIC_IAR3 = 0xffff5fff;					// PORTF IntA -> ID5

	// assign ISRs to interrupt vectors
	register_handler(ik_ivg12, PORTF_IntA_ISR);	// PORTF_IntA_ISR -> IVG 12

	// enable Timer0 and PORTF IntA interrupt
	*pSIC_IMASK = 0x08080000;
}

//--------------------------------------------------------------------------//

void LED_Init()
{
	int temp;
	
	temp = *pPORTF_FER;
	temp++;
#if (__SILICON_REVISION__ < 0x0001)
	*pPORTF_FER = 0x0000;
	*pPORTF_FER = 0x0000;
#else
	*pPORTF_FER = 0x0000;
#endif

	*pPORTFIO_INEN		= 0x0004;		// Pushbuttons 
	*pPORTFIO_DIR		= 0x0FC0;		// LEDs
	*pPORTFIO_EDGE		= 0x0004;
	*pPORTFIO_MASKA		= 0x0004;
	*pPORTFIO_SET 		= 0x0FC0;
	*pPORTFIO_CLEAR		= 0x0FC0;
};

//--------------------------------------------------------------------------//
// Function:	main														//
//--------------------------------------------------------------------------//
void main(void)
{
	int c;
	
	LED_Init();
	Init_Interrupts();
	
	UART_initialize(115200);
	UART_putc('H');
	UART_putc('e');
	UART_putc('l');
	UART_putc('l');
	UART_putc('o');
	
	for (;;)
	{
		c = UART_getc();
		if (c =='q') {
			*pPORTFIO_SET = 0x0040 | 0x0080 | 0x0100 | 0x0200 | 0x0400 | 0x0800;
		} else if (c == 'w') {
			*pPORTFIO_CLEAR =0x0FC4;
		}
		UART_putc(c);
	};
	while(1);
}

