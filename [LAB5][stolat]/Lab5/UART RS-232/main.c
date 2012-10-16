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
#include <stdlib.h>

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
int maska=0;
int zapalone=0;
EX_INTERRUPT_HANDLER(Timer0_ISR)
{
	*pPORTFIO_CLEAR = 0x0FC4;
	
}
EX_INTERRUPT_HANDLER(PORTF_IntA_ISR)
{
	UART_puts("Wcisnieto przycisk 1");
	UART_putc(13);
	UART_putc(10);
	*pPORTFIO_CLEAR = 0x0FC4;
}
void Init_Timers(void)
{
	*pTIMER0_CONFIG		= 0x0019;
	*pTIMER0_PERIOD		= 0x00800000;
	*pTIMER0_WIDTH		= 0x00400000;
	*pTIMER_ENABLE		= 0x0001;
}
void Init_Interrupts(void)
{
	// assign core IDs to interrupts
	*pSIC_IAR0 = 0xffffffff;
	*pSIC_IAR1 = 0xffffffff;
	*pSIC_IAR2 = 0xffff4fff;					// Timer0 -> ID4; 
	*pSIC_IAR3 = 0xffff5fff;					// PORTF IntA -> ID5

	// assign ISRs to interrupt vectors
	register_handler(ik_ivg11, Timer0_ISR);		// Timer0 ISR -> IVG 11
	register_handler(ik_ivg12, PORTF_IntA_ISR);	// PORTF_IntA_ISR -> IVG 12

	// enable Timer0 and PORTF IntA interrupt
	*pSIC_IMASK = 0x08080000;
}
void main(void)
{
	int c;
	
	LED_Init();
	
	//Init_Timers();
	//Init_Interrupts();
	
	UART_initialize(115200);
	//UART_putc('H');
	//UART_putc('e');
	//UART_putc('l');
	//UART_putc('l');
	//UART_putc('o');
	
	*pPORTFIO_TOGGLE=zapalone;
	//register_handler(ik_ivg12,PORTF_IntA_ISR);
	for (;;)
	{
		c = UART_getc();
		if((c>='1')&&(c<='6')){
			maska=0x1000>>(c-'0');
			if(zapalone&maska){
				zapalone=zapalone&(~maska);
				*pPORTFIO_TOGGLE=zapalone;
				UART_puts("Zgaszono diode ");
				UART_putc(c);
				UART_putc(13);
				UART_putc(10);
			}else{
				zapalone=zapalone|maska;
				*pPORTFIO_TOGGLE=zapalone;
				UART_puts("Zapalono diode ");
				UART_putc(c);
				UART_putc(13);
				UART_putc(10);
			}
		}
		/*while(*pPORTFIO_INEN){
			UART_putc(*pPORTFIO_INEN%10);
			*pPORTFIO_INEN/=10;
		}*/
		//UART_putc(0);
		//char str[10];
		//itoa(*pPORTFIO_INEN,str,10);
		//UART_puts(str);
		//UART_putc(c);
		//*pPORTFIO_TOGGLE = 0x0040 | 0x0080 | 0x0100 | 0x0200 | 0x0400 | 0x0800;
	};
	//while(1);
}
