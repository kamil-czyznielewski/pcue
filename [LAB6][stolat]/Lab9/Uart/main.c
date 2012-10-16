// *****************************************************************************/

#include "ccblkfn.h"
#include "sysreg.h"
#include "Uart0.h"
#include "Uart1.h"

#include <cDEFBF537.h>
#include <sys/exception.h>
#include "ccblkfn.h"
#include "sysreg.h"
#include <stdarg.h>
#include <string.h> 
#include <stdio.h>

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
}

struct TypDane{
	char typ[7];
	char liczba[20];
	char szerokosc[20];
	char wysokosc[20];
	char szerokoscT[2];
	char wysokoscT[2];
	char satelity[10];
	//int satelity;
};

void parsuj(char* dane,struct TypDane* sparsowaneDane){
	int i=0,nr;
	char str[10];
	//typ
	nr=0;
	while((dane[i]!=',')&&(dane[i]!=0)){
		sparsowaneDane->typ[nr]=dane[i];
		++i;
		++nr;
	}
	sparsowaneDane->typ[nr]=0;
	if(dane[i]!=0){
		++i;
	}
	//liczba
	nr=0;
	while((dane[i]!=',')&&(dane[i]!=0)){
		sparsowaneDane->liczba[nr]=dane[i];
		++i;
		++nr;
		if(sparsowaneDane->liczba[0]=='0'){
			--nr;
		}
	}
	sparsowaneDane->liczba[nr]=0;
	if(dane[i]!=0){
		++i;
	}
	//szerokosc
	nr=0;
	while((dane[i]!=',')&&(dane[i]!=0)){
		sparsowaneDane->szerokosc[nr]=dane[i];
		++i;
		++nr;
		if(sparsowaneDane->szerokosc[0]=='0'){
			--nr;
		}
	}
	sparsowaneDane->szerokosc[nr]=0;
	if(dane[i]!=0){
		++i;
	}
	nr=0;
	while((dane[i]!=',')&&(dane[i]!=0)){
		sparsowaneDane->szerokoscT[nr]=dane[i];
		++i;
		++nr;
	}
	sparsowaneDane->szerokoscT[nr]=0;
	if(dane[i]!=0){
		++i;
	}
	//wysokosc
	nr=0;
	while((dane[i]!=',')&&(dane[i]!=0)){
		sparsowaneDane->wysokosc[nr]=dane[i];
		++i;
		++nr;
		if(sparsowaneDane->wysokosc[0]=='0'){
			--nr;
		}
	}
	sparsowaneDane->wysokosc[nr]=0;
	if(dane[i]!=0){
		++i;
	}
	nr=0;
	while((dane[i]!=',')&&(dane[i]!=0)){
		sparsowaneDane->wysokoscT[nr]=dane[i];
		++i;
		++nr;
	}
	sparsowaneDane->wysokoscT[nr]=0;
	if(dane[i]!=0){
		++i;
	}
	//satelity
	nr=0;
	while((dane[i]!=',')&&(dane[i]!=0)){
		sparsowaneDane->satelity[nr]=dane[i];
		++i;
		++nr;
	}
	sparsowaneDane->satelity[nr]=0;
	if(dane[i]!=0){
		++i;
	}
	//
	//$GPGGA,090000.00,5129.95764936,N,00007.56974599,W,1,05,2.87,160.00,M,-21.3213,M,,*64
}

void wyswietlDane(char* dane){
	struct TypDane d;
	parsuj(dane,&d);
	//
	//UART0_puts(d.typ);
	UART0_puts("szerokosc: ");
	UART0_puts(d.szerokosc);
	UART0_puts(" ");
	UART0_puts(d.szerokoscT);
	UART0_puts(", wysokosc: ");
	UART0_puts(d.wysokosc);
	UART0_puts(" ");
	UART0_puts(d.wysokoscT);
	UART0_puts(", satelity: ");
	UART0_puts(d.satelity);
	UART0_puts(", czas: ");
	UART0_puts(d.liczba);
	//
	UART0_putc(10);
	UART0_putc(13);
}
//--------------------------------------------------------------------------//
// Function:	main														//
//--------------------------------------------------------------------------//
void main(void)
{
	
	LED_Init();
	
	UART0_initialize(115200);
	//UART0_putc('H');
	//UART0_putc('e');
	//UART0_putc('l');
	//UART0_putc('l');
	//UART0_putc('o');
	UART0_putc(10);
	UART0_putc(13);
	for (;;)
	{
		UART0_getc();
		wyswietlDane("$GPGGA,090000.00,5129.95764936,N,00007.56974599,W,1,05,2.87,160.00,M,-21.3213,M,,*64");
		//UART0_putc(c);
		*pPORTFIO_TOGGLE = 0x0040 | 0x0080 | 0x0100 | 0x0200 | 0x0400 | 0x0800;
	};
	//while(1);
	
}

