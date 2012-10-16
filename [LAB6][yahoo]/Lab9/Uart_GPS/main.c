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

void parse_stateful(char*, int); //parser ramki, dzia�a jak maszyna stan�w, pobiera analizowany ci�g i jego d�ugo��

//--------------------------------------------------------------------------//
// Function:	main														//
//--------------------------------------------------------------------------//
void main(void)
{
	char c;
	char buffer[300]; //odbierana ramka (teoretycznie maksymalna d�ugos� ramki-zdania to 80 znak�w + $ na pocz�tku + CR + LF, ale kto by tego przestrzega�?)
	int wptr=0; //wska�nik na miejsce zapisu w ramce
	
	UART0_initialize(115200); //ustawiamy opcje interfejsu szeregowego 0

	//g��wna p�tla programu
	while (1)
	{
		//odczytywanie zdania
		while (1)
		{
			//je�eli odebrano co najmniej dwa znaki i dwa ostatnio odebrane to CR i LF, to znaczy, �e kto� nam poda� ca�� ramk� i mo�emy przej�� do jej analizy
			if (wptr>1 && buffer[wptr-2]=='\r' && buffer[wptr-1]=='\n')
				break;
			
			//odczytaj znak i dopisz go do zawartosci bufora
			c = UART0_getc();
			buffer[wptr]=c;
			wptr++;
			
			//wykryj przepe�nienie bufora, je�eli takie nast�pi�o
			if (wptr>=300)
			{
				UART0_puts("Buffer overflow; erasing\r\n");
				wptr=0;
				continue;
			}
		}
		
		//po zako�czeniu odbierania ramki trzeba jeszcze oznaczy� jej koniec
		buffer[wptr]=0;
		
		//przeka� ramk� i jej d�ugo�� do funkcji parsuj�cej
		parse_stateful(buffer, wptr);
		
		//oznacz, �e bufor jest pusty
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
 * Do tych zmiennych b�d� zapisywane dane wyd�ubane z odebranej ramki NMEA.
 * 
 * Analizator dzia�a jak maszyna stan�w, tj. w ka�dej chwili odpowiadaj�cej
 * analizie jednej litery z ramki b�dzie znajdowa� si� w pewnym stanie.
 * W zale�no�ci od tego stanu co� z analizowan� liter� zrobi...
 * 
 * Stany:
 * 0 - pocz�tek ramki, stan ko�czy si� po napotkaniu $
 * 1 - talker & sentence identifier (pomijany), przej�cie do nast�pnego stanu nast�puje po napotkaniu przecinka
 * 2 - time, stan ko�czy si� po napotkaniu przecinka
 * 3 - latitude value, stan ko�czy si� przecinkiem
 * 4 - latitude dir, stan ko�czy si� po odczytaniu jednego znaku lub napotkaniu przecinka
 * 5 - longitude value, ko�czony po napotkaniu przecinka
 * 6 - longitude dir, ko�czony po odczycie jednego znaku lub napotkaniu przecinka
 * 7 - quality (pomijany), stan ko�czy si� po napotkaniu przecinak
 * 8 - satellite count, stan ko�czy si� po napotkaniu przecinka
 * Pozosta�e pola z ramki nas nie interesuj� wi�c wi�cej stan�w nie ma
 */

//a do tych zmiennych zapiszemy sobie te elementy ramki, kt�re s� dla nas wa�ne
char time[200];
int time_c = 0; //wska�nik miejsca zapisu do time
char satellite[200];
int satellite_c = 0; //wska�nik miejsca zapisu do satellite
char latitude[200];
char latitude_c = 0; //wska�nik miejsca zapisu do latitude
char longitude[200];
char longitude_c = 0; //wska�nik miejsca zapisu do longitude

//Yancc, czyli co� jak analizator sk�adniowy
void parse_stateful(char *buffer, int length)
{
	//resetowanie stanu
	int stage = 0;
	
	//resetowanie d�ugo�ci ci�g�w
	time_c = 0;
	satellite_c = 0;
	latitude_c = 0;
	longitude_c = 0;
	
	//parsownaie kolejnych znak�w
	int i;
	for (i=0; i<length; i++)
	{
    //odczytanie znaku z bufora
		char c = buffer[i];
		
    //co� z nim robimy w zale�no�ci od stanu
    switch (stage)
		{
			case 0: //$ na pocz�tku ramki - przechodzimy dalej; brak $ = b��d sk�adni
				if (c == '$')
					stage++;
				else 
					UART0_puts("syntax error 0\r\n");
				break;
			
			case 1: //przecinek to przej�cie do nast�pnego stanu, ka�dy inny znak jest pomijany (talker i sentence identifier)
				if (c == ',')
					stage++;
				break;
			
			case 2: //przecinek to przej�cie do nast�pnego stanu, ka�dy inny znak jest dopisywany do time
				if (c == ',')
        {
					stage++;
          time[time_c++]=0;
        }
				else
					time[time_c++]=c;
				break;
				
			case 3: //przecinek to przej�cie do nast�pnego stanu, ka�dy inny znak jest dopisywany do latitude
				if (c == ',')
					stage++;
				else
					latitude[latitude_c++]=c;
				break;
				
			case 4: //przecinek to przej�cie do nast�pnego stanu, ka�dy inny znak (a pewnie b�dzie tylko jeden) jest dopisywany do latitude jako kierunek (N lub S, o ile dobrze pami�tam)
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
			
			case 7: //wszystkie znaki z GPS quality ignorujemy, przecinek to nast�pny stan
				if (c == ',')
					stage++;
				break;
			
			case 8: //musimy jeszcze odczyta� czas
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

  //no i wreszcie po przejechaniu ca�ej ramki wypisujemy to co�my odczytali
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
