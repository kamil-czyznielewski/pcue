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

void Init_Interrupts(void)
{
	// assign core IDs to interrupts
	*pSIC_IAR0 = 0xffffffff;
	*pSIC_IAR1 = 0xffffffff;
	*pSIC_IAR2 = 0xffff4fff;					// Timer0 -> ID4; 
	*pSIC_IAR3 = 0xffff5fff;					// PORTF IntA -> ID5

	// assign ISRs to interrupt vectors
	register_handler(ik_ivg12, PORTF_IntA_ISR);	// PORTF_IntA_ISR -> IVG 12

	// enable Timer0 and PORTF IntA interrupt
	*pSIC_IMASK = 0x08080000;
}

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
	UART_putc('\r');
	UART_putc('\n');
	
	for (;;)
	{
		c = UART_getc();
		
		if (c == '1')
		{
			*pPORTFIO_TOGGLE = 0x0040;
		}
		else if (c == '2')
		{
			*pPORTFIO_TOGGLE = 0x0080;				
		}
		else if (c == '3')
		{
			*pPORTFIO_TOGGLE = 0x0100;				
		}
		else if (c == '4')
		{
			*pPORTFIO_TOGGLE = 0x0200;				
		}
		else if (c == '5')
		{
			*pPORTFIO_TOGGLE = 0x0400;				
		}
		else if (c == '6')
		{
			*pPORTFIO_TOGGLE = 0x0800;			
		}			
	};
}

