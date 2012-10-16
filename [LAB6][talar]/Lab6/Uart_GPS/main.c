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

void parse_stateful(char*, int);

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
	char c;
	char buffer[300];
	int wptr=0;
	
	LED_Init();
	
	UART0_initialize(115200);

	UART1_initialize(19200);
	
	while(1) {
		for (;;)
		{
			if (UART1_checkc())
			{
				if (wptr>2 && buffer[wptr-1]=='\r' || buffer[wptr-1]=='\n')
					break;
			
				c = UART1_getc();
				
				buffer[wptr]=c;
				wptr++;
			
				if (wptr>=300)
				{
					UART0_puts("Buffer overflow; erasing\r\n");
					wptr=0;
					continue;
				}
			
				*pPORTFIO_TOGGLE = 0x0040 | 0x0080 | 0x0100 | 0x0200 | 0x0400 | 0x0800;
			};

		};
	
		buffer[wptr]=0;
		
		//przeka¿ ramkê i jej d³ugoœæ do funkcji parsuj¹cej
		parse_stateful(buffer, wptr);
		
		//oznacz, ¿e bufor jest pusty
		wptr=0;
	}	
}

char time[200];
int time_c = 0; //wskaŸnik miejsca zapisu do time
char satellite[200];
int satellite_c = 0; //wskaŸnik miejsca zapisu do satellite
char latitude[200];
char latitude_c = 0; //wskaŸnik miejsca zapisu do latitude
char longitude[200];
char longitude_c = 0; //wskaŸnik miejsca zapisu do longitude


void parse_stateful(char *buffer, int length)
{
	//resetowanie stanu
	int stage = 0;
	
	//resetowanie d³ugoœci ci¹gów
	time_c = 0;
	satellite_c = 0;
	latitude_c = 0;
	longitude_c = 0;
	
	//parsownaie kolejnych znaków
	int i;
	for (i=0; i<length; i++)
	{
    //odczytanie znaku z bufora
		char c = buffer[i];
		
    //coœ z nim robimy w zale¿noœci od stanu
    switch (stage)
		{
			case 0: //$ na pocz¹tku ramki - przechodzimy dalej; brak $ = b³¹d sk³adni
				if (c == '$')
					stage++;
				else 
					UART0_puts("");//"syntax error 0\r\n");
				break;
			
			case 1: //przecinek to przejœcie do nastêpnego stanu, ka¿dy inny znak jest pomijany (talker i sentence identifier)
				if (c == ',')
					stage++;
				break;
			
			case 2: //przecinek to przejœcie do nastêpnego stanu, ka¿dy inny znak jest dopisywany do time
				if (c == ',')
        {
					stage++;
          time[time_c++]=0;
        }
				else
					time[time_c++]=c;
				break;
				
			case 3: //przecinek to przejœcie do nastêpnego stanu, ka¿dy inny znak jest dopisywany do latitude
				if (c == ',')
					stage++;
				else
					latitude[latitude_c++]=c;
				break;
				
			case 4: //przecinek to przejœcie do nastêpnego stanu, ka¿dy inny znak (a pewnie bêdzie tylko jeden) jest dopisywany do latitude jako kierunek (N lub S, o ile dobrze pamiêtam)
				if (c == ',')
        {
					stage++;
          latitude[latitude_c++]=0;
        }
				else
					latitude[latitude_c++]=c;
				break;
	
			case 5: //wszystko analogicznie jak dla latitude
				if (c == ',')
					stage++;
				else
					longitude[longitude_c++]=c;
				break;				
				
			case 6: //wszystko analogicznie jak dla latitude
				if (c == ',')
        {
					stage++;
          longitude[longitude_c++]=0;
        }
				else
					longitude[longitude_c++]=c;
				break;
			
			case 7: //wszystkie znaki z GPS quality ignorujemy, przecinek to nastêpny stan
				if (c == ',')
					stage++;
				break;
			
			case 8: //musimy jeszcze odczytaæ czas
				if (c == ',')
        {
					stage++;
          satellite[satellite_c++]=0;
        }
				else
					satellite[satellite_c++]=c;
				break;
		}
	}

  //no i wreszcie po przejechaniu ca³ej ramki wypisujemy to coœmy odczytali
	UART0_puts("--------------------------------------------\r\n");
	
	UART0_puts("Czas: ");
	UART0_puts(time);
	UART0_puts("\r\n");
	
	UART0_puts("Pozycja: ");
	UART0_puts(latitude);
	UART0_puts(", ");
	UART0_puts(longitude);
	UART0_puts("\r\n");
	
	UART0_puts("Satelity: ");
	UART0_puts(satellite);
	UART0_puts("\r\n");	
}