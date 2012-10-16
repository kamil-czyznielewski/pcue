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

void Init_Flags(void)
{
	int temp;	
	
	temp = *pPORTF_FER; //nie mam najmniejszego pojêcia po co to robimy oO
	temp++;
#if (__SILICON_REVISION__ < 0x0001) //panowie (a mo¿e i panie?) z Analog Devices skopali coœ w starej wersji BF537, wiêc je¿eli tak¹ mamy to trzeba powtórzyæ dwa razy pen¹ operacjê
	*pPORTF_FER = 0x0000; //wszystkie piny PORTF maj¹ pe³niæ funkcjê portu I/O a nie jakichœ tam innych peryferiów typu UART czy CAN (FER=function enable register)
	*pPORTF_FER = 0x0000;
#else
	*pPORTF_FER = 0x0000;
#endif

	*pPORTFIO_INEN		= 0x0004;		// Pin pod³¹czony do jednego z przycisków (czwartego) ma byæ wejœciem (INEN = Input Enable)
	*pPORTFIO_DIR	    = 0x0FC0;		// Piny pod³¹czone do diod (diód?) maj¹ byæ wyjœciami (DIRECTION=1 (output))
	*pPORTFIO_EDGE		= 0x0004;   // Pin pod³¹czony do przycisku ma byæ aktywowany (odpowiedni bit ustawiony w PORTFIO) zboczem
	*pPORTFIO_MASKA		= 0x0004;   // Odmaskowywujemy ten pin na potrzeby generacji przerwañ (?)
	*pPORTFIO_SET 		= 0x0FC0;   // Zapalamy wszystkie diody
	*pPORTFIO_CLEAR		= 0x0FC0;   // Gasimy wszystkie diody
}

//--------------------------------------------------------------------------//

void Init_Interrupts(void)
{
	// assign core IDs to interrupts
	*pSIC_IAR0 = 0xffffffff;
	*pSIC_IAR1 = 0xffffffff;
	*pSIC_IAR3 = 0xffff5fff;					// PORTF IntA -> ID5

	// assign ISRs to interrupt vectors
	register_handler(ik_ivg12, PORTF_IntA_ISR);	// PORTF_IntA_ISR -> IVG 12

	// enable Timer0 and PORTF IntA interrupt
	*pSIC_IMASK = 0x08080000;//dwa bity ustawione w tym rejestrze odpowiadaj¹ za aktywacjê przerwañ od timera i od przycisku, tylko nie wiadomo, który jest od czego; zostaj¹ oba
}

//--------------------------------------------------------------------------//

//procedura obs³ugi przerwania od przycisku
EX_INTERRUPT_HANDLER(PORTF_IntA_ISR)
{
	// confirm interrupt handling and clear flags
	*pPORTFIO_CLEAR = 0x0FC4;

  //wysy³amy coœ weso³ego na terminal
  UART_puts("Dziala!\r\n");
}

//--------------------------------------------------------------------------//
// Function:	main														//
//--------------------------------------------------------------------------//
void main(void)
{
	int c;
	
  //inicjalizacje
	Init_Interrupts();
	LED_Init();
  UART_initialize(115200);
	
  //jakiœ tam komunikat powitalny
	UART_putc('H');
	UART_putc('e');
	UART_putc('l');
	UART_putc('l');
	UART_putc('o');
	
	for (;;)
	{
    //odbieramy znak i odbijamy go na terminal
		c = UART_getc();
		UART_putc(c);
		
    //w zale¿noœci od tego co przysz³o, prze³¹czamy stan jednej z diod
    switch (c)
		{
			case '1':
				*pPORTFIO_TOGGLE = 0x0040;
				break;
			case '2':
				*pPORTFIO_TOGGLE = 0x0080;
				break;
			case '3':
				*pPORTFIO_TOGGLE = 0x0100;
				break;
			case '4':
				*pPORTFIO_TOGGLE = 0x0200;
				break;
			case '5':
				*pPORTFIO_TOGGLE = 0x0400;
				break;
			case '6':
				*pPORTFIO_TOGGLE = 0x0800;
				break;
		}
	}
}

