//Plik w projekcie BF537, main.c


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

    *pPORTFIO_INEN        = 0x0004;        // Pushbuttons
    *pPORTFIO_DIR        = 0x0FC0;        // LEDs
    *pPORTFIO_EDGE        = 0x0004;
    *pPORTFIO_MASKA        = 0x0004;
    *pPORTFIO_SET         = 0x0FC0;
    *pPORTFIO_CLEAR        = 0x0FC0;
};

//--------------------------------------------------------------------------//
// Function:    main                                                        //
//--------------------------------------------------------------------------//
void main(void)
{
   
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
    UART0_putc('\n');
    UART0_putc('\r');

    UART1_initialize(19200);
    //UART1_putc('F');
   
   
   
    int c;

        //char *nmea = "$GPGGA,090000.10,5129.95764936,N,00007.56974599,W,1,05,2.87,160.00,M,-21.3213,M,,*65";
        char *nmea = "$GPGGA,070000.15,5129.95764936,N,00007.56974599,W,1,26,2.87,160.00,M,-21.3213,M,,*60";

       char tmp[1];
       char buffer[40];
       char napis[250];
       int i,j;

       strcpy(buffer," ");
       strcpy(napis, "");

       // pobieramy 'naglowek'
       for (i = 0; i<6; i++)
       {
               tmp[0] = nmea[i];
               strcat(buffer, tmp);
       }


       if(strcmp(buffer,"$GPGGA"))
       {
               //printf("Informacje z GPS\n");
               UART0_puts("Informacje z GPS\n\r");
               strcpy(buffer,"");

               // bo $GPGGA to indeksy od 0 do 6
               i= 7;

               while (nmea[i] != ',')
               {
                       tmp[0] = nmea[i++];
                       strcat(buffer, tmp);
               }

              
               sprintf(napis, "Czas RTC: %c%c:%c%c:%c%cs %c%cms\n\r",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[7],buffer[8]);
               UART0_puts(napis);
               strcpy(napis, "");
               sprintf(napis, "Informacje o polozeniu: \n\r");
               UART0_puts(napis);
               strcpy(napis, "");
               strcpy(buffer,"");

               i = i + 1;

               while (nmea[i] != ',')
               {
                       tmp[0] = nmea[i++];
                       strcat(buffer, tmp);
               }

               sprintf(napis, "Szerokosc geog.: %c%c stopni %c%c%c%c%c%c%c%c%c%c min",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12]);
               UART0_puts(napis);
               strcpy(napis, "");
               strcpy(buffer,"");

               i = i + 1;

               while (nmea[i] != ',')
               {
                       tmp[0] = nmea[i++];
                       strcat(buffer, tmp);
               }

               // N lub S
               sprintf(napis," %c\n\r",buffer[0]);
               UART0_puts(napis);
               strcpy(napis, "");
               strcpy(buffer,"");

               i = i + 1;

               while (nmea[i] != ',')
               {
                       tmp[0] = nmea[i++];
                       strcat(buffer, tmp);
               }

               sprintf(napis, "Dlugosc geog.  : %c%c%c stopni %c%c%c%c%c%c%c%c%c min",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12]);
               UART0_puts(napis);
               strcpy(napis, "");
               strcpy(buffer,"");

               i = i + 1;

               while (nmea[i] != ',')
               {
                       tmp[0] = nmea[i++];
                       strcat(buffer, tmp);
               }

               // W lub E
               sprintf(napis, " %c\n\r",buffer[0]);
               UART0_puts(napis);
               strcpy(napis, "");

               strcpy(buffer," ");

               i = i + 1;

               while (nmea[i] != ',')
               {
                       tmp[0] = nmea[i++];
                       strcat(buffer, tmp);
               }

               // tutaj by³a informacja nieprzydatna dla u¿ytkownika
               strcpy(buffer,"");

               i = i + 1;

               while (nmea[i] != ',')
               {
                       tmp[0] = nmea[i++];
                       strcat(buffer, tmp);
               }

               if (buffer[0] == '0')
               {
                       sprintf(napis, "Liczba satelit : %c\n\r",buffer[1]);
               }
               else
                  {
                        sprintf(napis, "Liczba satelit : %c%c\n\r",buffer[0],buffer[1]);
                       
                  }
                  UART0_puts(napis);
               strcpy(napis, "");
               strcpy(buffer, "");
              
               i = i + 1;

               while (nmea[i] != ',')
               {
                       tmp[0] = nmea[i++];
                       strcat(buffer, tmp);
               }

               sprintf(napis, "Horizontal Dilution of precision : %c.%c\n\r",buffer[0],buffer[2]);
               UART0_puts(napis);
               strcpy(napis, "");
               strcpy(buffer, "");
              
               i = i + 1;

               while (nmea[i] != ',')
               {
                       tmp[0] = nmea[i++];
                       strcat(buffer, tmp);
               }

               sprintf(napis, "Units of antenna altitude, meters : %c.%c\n\r",buffer[0],buffer[2]);
               UART0_puts(napis);
               strcpy(napis, "");

       }
   
   
   
   
   
    for (;;)
    {
        /*if (UART1_checkc())
        {
            c = UART1_getc();
            UART0_putc(c);
            *pPORTFIO_TOGGLE = 0x0040 | 0x0080 | 0x0100 | 0x0200 | 0x0400 | 0x0800;
        };*/
        if (UART0_checkc())
        {
            c = UART0_getc();
            //UART1_putc(c);           
            UART0_putc('#');
            UART0_putc(c);
            UART0_putc('\n');
            UART0_putc('\r');
            *pPORTFIO_TOGGLE = 0x0040 | 0x0080 | 0x0100 | 0x0200 | 0x0400 | 0x0800;
        };
    };
    while(1);
   
}
