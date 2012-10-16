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


struct Data{
	char type[7];
	char time[20];
	char width[20];
	char height[20];
	char widthT[2];
	char heightT[2];
	char satelites[10];
};

void parse(char* data,struct Data* parsedData){
	int i=0,nr;
	char str[10];
	//typ
	nr=0;
	while((data[i]!=',')&&(data[i]!=0)){
		parsedData->type[nr]=data[i];
		++i;
		++nr;
	}
	parsedData->type[nr]=0;
	if(data[i]!=0){
		++i;
	}
	//czas
	nr=0;
	while((data[i]!=',')&&(data[i]!=0)){
		parsedData->time[nr]=data[i];
		++i;
		if (nr==1 || nr==4)
		{
			++nr;
			parsedData->time[nr]=':';	
		}
		
		++nr;
		if(parsedData->time[0]=='0'){
			--nr;
		}
	}
	parsedData->time[nr]=0;
	if(data[i]!=0){
		++i;
	}
	//szerokosc
	nr=0;
	while((data[i]!=',')&&(data[i]!=0)){
		parsedData->width[nr]=data[i];
		++i;
		++nr;
		if(parsedData->width[0]=='0'){
			--nr;
		}
	}
	parsedData->width[nr]=0;
	if(data[i]!=0){
		++i;
	}
	nr=0;
	while((data[i]!=',')&&(data[i]!=0)){
		parsedData->widthT[nr]=data[i];
		++i;
		++nr;
	}
	parsedData->widthT[nr]=0;
	if(data[i]!=0){
		++i;
	}
	//wysokosc
	nr=0;
	while((data[i]!=',')&&(data[i]!=0)){
		parsedData->height[nr]=data[i];
		++i;
		++nr;
		if(parsedData->height[0]=='0'){
			--nr;
		}
	}
	parsedData->height[nr]=0;
	if(data[i]!=0){
		++i;
	}
	nr=0;
	while((data[i]!=',')&&(data[i]!=0)){
		parsedData->heightT[nr]=data[i];
		++i;
		++nr;
	}
	parsedData->heightT[nr]=0;
	if(data[i]!=0){
		++i;
	}
	//trzeba ominac fixa
	while((data[i]!=',')&&(data[i]!=0)){++i;}
	
	if(data[i]!=0){
		++i;
	}
	
	//satelity
	nr=0;
	while((data[i]!=',')&&(data[i]!=0)){
		parsedData->satelites[nr]=data[i];
		++i;
		++nr;
	}
	parsedData->satelites[nr]=0;
	if(data[i]!=0){
		++i;
	}

}

showData(char* data){
	struct Data d;
	parse(data,&d);
	
	//UART0_puts(d.type);
	UART0_puts("Szer: ");
	UART0_puts(d.width);
	UART0_puts(" ");
	UART0_puts(d.widthT);
	UART0_puts(", Wys: ");
	UART0_puts(d.height);
	UART0_puts(" ");
	UART0_puts(d.heightT);
	UART0_puts(", Satelity: ");
	UART0_puts(d.satelites);
	UART0_puts(", Czas: ");
	UART0_puts(d.time);
	
	UART0_putc(10);
	UART0_putc(13);
}

char dane[85];
int i = 0;
bool start;

//--------------------------------------------------------------------------//
// Function:	main														//
//--------------------------------------------------------------------------//
void main(void)
{
	int c;
	int i = 0;
	bool start = false;
	
	/*
	LED_Init();
	
	UART0_initialize(115200);
	UART0_putc('H');
	UART0_putc('e');
	UART0_putc('l');
	UART0_putc('l');
	UART0_putc('o');
	
	for (;;)
	{
		c = UART0_getc();
		UART0_putc(c);
		*pPORTFIO_TOGGLE = 0x0040 | 0x0080 | 0x0100 | 0x0200 | 0x0400 | 0x0800;
	};
	while(1);
	*/
	
	LED_Init();
	
	UART0_initialize(115200);
	UART0_putc('H');
	UART0_putc('e');
	UART0_putc('l');
	UART0_putc('l');
	UART0_putc('o');

	UART1_initialize(19200);
	//UART1_putc('F');
	
	
	for (;;)
	{
		if (UART1_checkc())
		//bierze dane z gps
		{
			c = UART1_getc();
			//UART0_putc(c);
			if (start) {
				if (i == 3 && c != 'G') {
					start = false;
					i = 0;
				}
				else {
					dane[i]=c;
					i++;
				}	
			}
			
			if (c=='$') {
				start = true;
				dane[i]=c;
				i++;	
			}
			
			
			
			if (c=='*' && start) {
				showData(dane);
				i = 0;
				start = false;
				}
				
			*pPORTFIO_TOGGLE = 0x0040 | 0x0080 | 0x0100 | 0x0200 | 0x0400 | 0x0800;
		};
		if (UART0_checkc())
		{
			c = UART0_getc();
			UART0_putc('#');
			UART0_putc(c);
			UART0_putc('\n');
            UART0_putc('\r');			
			*pPORTFIO_TOGGLE = 0x0040 | 0x0080 | 0x0100 | 0x0200 | 0x0400 | 0x0800;
		};
	};
	while(1);
	
}


