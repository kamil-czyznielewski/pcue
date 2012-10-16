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
#include <stdlib.h>
#include <stdio.h>

//--------------------------------------------------------------------------//

void parse_stateful(char*, int); //parser ramki, dzia³a jak maszyna stanów, pobiera analizowany ci¹g i jego d³ugoœæ

//--------------------------------------------------------------------------//
// Function:	main														//
//--------------------------------------------------------------------------//
void main(void)
{
	char c;
	char buffer[300]; //odbierana ramka (teoretycznie maksymalna d³ugosæ ramki-zdania to 80 znaków + $ na pocz¹tku + CR + LF, ale kto by tego przestrzega³?)
	int wptr=0; //wskaŸnik na miejsce zapisu w ramce
	
	UART0_initialize(115200); //ustawiamy opcje interfejsu szeregowego 0

	//g³ówna pêtla programu
	while (1)
	{
		//odczytywanie zdania
		while (1)
		{
			//je¿eli odebrano co najmniej dwa znaki i dwa ostatnio odebrane to CR i LF, to znaczy, ¿e ktoœ nam poda³ ca³¹ ramkê i mo¿emy przejœæ do jej analizy
			if (wptr>1 && buffer[wptr-2]=='\r' && buffer[wptr-1]=='\n')
				break;
			
			//odczytaj znak i dopisz go do zawartosci bufora
			c = UART0_getc();
			buffer[wptr]=c;
			wptr++;
			
			//wykryj przepe³nienie bufora, je¿eli takie nast¹pi³o
			if (wptr>=300)
			{
				UART0_puts("Buffer overflow; erasing\r\n");
				wptr=0;
				continue;
			}
		}
		
		//po zakoñczeniu odbierania ramki trzeba jeszcze oznaczyæ jej koniec
		buffer[wptr]=0;
		
		//przeka¿ ramkê i jej d³ugoœæ do funkcji parsuj¹cej
		parse_stateful(buffer, wptr);
		
		//oznacz, ¿e bufor jest pusty
		wptr=0;
	};
	
}

//--------------------------------------------------------------------------//

/*
stages of parsing NMEA sentence:
 0 - $ at the beginning
 1 - sentence and talker identifier skipping
 2 - time
 3 - latitude value
 4 - latitude dir
 5 - longitude
 6 - longitude dir
 7 - quality skipping
 8 - satellite count
*/

/*
 * Do tych zmiennych bêd¹ zapisywane dane wyd³ubane z odebranej ramki NMEA.
 * 
 * Analizator dzia³a jak maszyna stanów, tj. w ka¿dej chwili odpowiadaj¹cej
 * analizie jednej litery z ramki bêdzie znajdowa³ siê w pewnym stanie.
 * W zale¿noœci od tego stanu coœ z analizowan¹ liter¹ zrobi...
 * 
 * Stany:
 * 0 - pocz¹tek ramki, stan koñczy siê po napotkaniu $
 * 1 - talker & sentence identifier (pomijany), przejœcie do nastêpnego stanu nastêpuje po napotkaniu przecinka
 * 2 - time, stan koñczy siê po napotkaniu przecinka
 * 3 - latitude value, stan koñczy siê przecinkiem
 * 4 - latitude dir, stan koñczy siê po odczytaniu jednego znaku lub napotkaniu przecinka
 * 5 - longitude value, koñczony po napotkaniu przecinka
 * 6 - longitude dir, koñczony po odczycie jednego znaku lub napotkaniu przecinka
 * 7 - quality (pomijany), stan koñczy siê po napotkaniu przecinak
 * 8 - satellite count, stan koñczy siê po napotkaniu przecinka
 * Pozosta³e pola z ramki nas nie interesuj¹ wiêc wiêcej stanów nie ma
 */

//a do tych zmiennych zapiszemy sobie te elementy ramki, które s¹ dla nas wa¿ne
char time[200];
int time_c = 0; //wskaŸnik miejsca zapisu do time
char satellite[200];
int satellite_c = 0; //wskaŸnik miejsca zapisu do satellite
char latitude[200];
char latitude_c = 0; //wskaŸnik miejsca zapisu do latitude
char longitude[200];
char longitude_c = 0; //wskaŸnik miejsca zapisu do longitude

//Yancc, czyli coœ jak analizator sk³adniowy
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
					UART0_puts("syntax error 0\r\n");
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
	
	UART0_puts("Time: ");
	UART0_puts(time);
	UART0_puts("\r\n");
	
	UART0_puts("Position: ");
	UART0_puts(latitude);
	UART0_puts(", ");
	UART0_puts(longitude);
	UART0_puts("\r\n");
	
	UART0_puts("Satellites: ");
	UART0_puts(satellite);
	UART0_puts("\r\n");	
}
