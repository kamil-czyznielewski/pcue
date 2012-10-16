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
};

FILE *fr;

//--------------------------------------------------------------------------//
// Function:	main														//
//--------------------------------------------------------------------------//
void main(void)
{
	int c;
	char *nmea= "$GPGGA,090000.30,5129.95764936,N,00007.56974599,W,1,05,2.87,160.00,M,-21.3213,M,,*67";
	char tmp[1];
	char buffer[40];
	
		
	int count = strlen(nmea);
	int i,j;
	
	strcpy(buffer," ");
	
	for(i = 0; i<6; i++){
		tmp[0] = nmea[i];
		strcat(buffer, tmp);						
	}
	
	if(strcmp(buffer,"$GPGGA")){
		
		printf("INIT GPS\n");
		strcpy(buffer," ");
		
		i= 7;
			
		while(nmea[i] != ','){
			tmp[0] = nmea[i++];
			strcat(buffer, tmp);
		}
		
		printf("Czas-RTC: %c%c:%c%c:%c%cs %c%cms\n",buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[8],buffer[9]);
		strcpy(buffer," ");
		
		i = i + 1;
		
		while(nmea[i] != ','){
			tmp[0] = nmea[i++];
			strcat(buffer, tmp);
		}
		
		printf("Szerokosc: %c%c stopni %c%c%c%c%c%c%c%c%c%c min",buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12],buffer[13]);
			
		strcpy(buffer," ");
		
		i = i + 1;
		
		while(nmea[i] != ','){
			tmp[0] = nmea[i++];
			strcat(buffer, tmp);
		}
		
		printf(" %c\n",buffer[1]);
		
		strcpy(buffer," ");
		
		i = i + 1;
		
		while(nmea[i] != ','){
			tmp[0] = nmea[i++];
			strcat(buffer, tmp);
		}
		
		printf("Dlugosc: %c%c%c stopni %c%c%c%c%c%c%c%c%c min",buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12],buffer[13]);
			
		strcpy(buffer," ");
		
		i = i + 1;
		
		while(nmea[i] != ','){
			tmp[0] = nmea[i++];
			strcat(buffer, tmp);
		}
		
		printf(" %c\n",buffer[1]);
		
		strcpy(buffer," ");
		
		i = i + 1;
		
		while(nmea[i] != ','){
			tmp[0] = nmea[i++];
			strcat(buffer, tmp);
		}
		
		strcpy(buffer," ");
		
		i = i + 1;
		
		while(nmea[i] != ','){
			tmp[0] = nmea[i++];
			strcat(buffer, tmp);
		}
		
		printf("Satelity: %c%c\n",buffer[1],buffer[2]);
	}
	
	LED_Init();
	
	UART0_initialize(115200);
	UART0_putc('H');
	UART0_putc('e');
	UART0_putc('l');
	UART0_putc('l');
	UART0_putc('o');

	UART1_initialize(19200);
	
	
	for (;;)
	{
		if (UART1_checkc())
		{
			c = UART1_getc();
			UART0_putc(c);
			*pPORTFIO_TOGGLE = 0x0040 | 0x0080 | 0x0100 | 0x0200 | 0x0400 | 0x0800;
		};
		if (UART0_checkc())
		{
			c = UART0_getc();
			UART1_putc(c);			
			*pPORTFIO_TOGGLE = 0x0040 | 0x0080 | 0x0100 | 0x0200 | 0x0400 | 0x0800;
		};
	};
	while(1);
	
}

