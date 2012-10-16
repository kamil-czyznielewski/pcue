/*----------------------------------------------------------------------------
 * Name:    Blinky.c
 * Purpose: LED Flasher
 * Note(s): possible defines set in "options for target - C/C++ - Define"
 *            __USE_LCD   - enable Output on LCD
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2008-2011 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include "Serial.h"

/*----------------------------------------------------------------------------
  Main Program
 *----------------------------------------------------------------------------*/
/*
 * Jak to dzia³a?
 *
 * 1) Funkcje odbioru znaku z UART0 i UART1 (Serial.c) zosta³y zmienione na nie
 *    blokuj¹ce i zwracaj¹ 0, gdy nie odebrano ¿adnego znaku
 * 2) Funckja inicjalizacji UARTu (Serial.c) zosta³a tak przerobiona, by
 *    inicjalizowaæ oba uarty na raz
 * 3) G³ówny program dzia³a tak, ¿e ka¿dy znak z UART0 jest przekierowywany na UART1
 *    a ka¿dy znak z UART1 - na UART0. W efekcie mo¿na przy u¿yciu komputera
 *    komunikwaæ siê z modu³em GSM.
 *
 *    Dodatkowo gdy na UART1 (do strony komputera) zostanie wykryty znak specjalny:
 *    '(' lub ')', to zostanie wywo³any odpowiedni algorytm (na p³ytce)
 *
 *    Dla '(' p³ytka spyta modu³ o stan sieci i przeka¿d odpowiedŸ komputerowi
 *    Dla ')' p³ytka za¿¹da odebrania SMSa z pocz¹tku kolejki oczekuj¹cych (SMSa nr 1)
 *    i przeœle jego treœæ komputerowi
 */

int main (void) {

  //J: zmienne pomocnicze
  int c;              //znak odebrany przez UART0 lub UART1
  char response[150]; //miejsce na odpowiedŸ modu³u
  int char_c=0;       //liczba znaków w odpowiedzi
  int nl_c=0;         //liczba linii w odpowiedzi

  SER_Init();                     /* UART0 and UART1 Initialization           */
  
  //J: w pêtli bêdziemy przekazywaæ znak po znaku od komputera do modu³u i odwrotnie, chyba ¿e z komputera dotrze do nas znak specjalny
  while (1) {
    //gdy jakiœ znak przyszed³ na UART0 (od modu³u)...
    if ((c = SER0_GetChar()) != 0) 
    {
      //je¿eli odebrano coœ na UART0, to przeœlij to na UART1
      SER1_PutChar(c);
    }

    //gdy jakiœ znak przyszed³ na UART1 (od komputera)...
    if ((c = SER1_GetChar()) != 0) 
    {
      switch (c)
      {
        //znak specjalny '(' powoduje sprawdzenie stanu sieci
        case '(':
          //podobno jesteœmy user-friendly
          SER1_PutStr("Sprawdzam stan zalogowania do sieci: \r\n");
          
          //wysy³amy komendê sprawdzenia stanu sieci
          SER0_PutStr("AT+CREG?\r\n");

          //czyœcimy bufor
          char_c=0;
          nl_c=0;

          //-------------------------------------------------------------------
          //Wydaje mi siê, ¿e odpowiedŸ na AT+CREG? ma 22 znaki (razem ze znakami
          //nowej linii i echem samej komendy), a cyfra oznaczaj¹ca status sieci
          //siedzi w 20 znaku, ale TRZEBA TO SPRAWDZIC NA SPRZÊCIE
		  //
		  //Przyk³ad:
		  //na UART0 leci:
		  //AT+CREG?<CR><LF>
		  //a w odpowiedzi (zgodnie z dokumentacj¹) powinniœmy dostaæ
		  //AT+CREG?<CR><LF>
		  //+CREG: 0,2<CR><LF>
          //-------------------------------------------------------------------

          //zbieramy ¿niwo
          //-------------------------------------------------------------------
          //TODO: sprawdziæ warunek zakoñczenia zapisywania danych do bufora
          //-------------------------------------------------------------------
          while (/*warunek stopu w oparciu o nl_c lub char_c typu: odbierz x linii lub odbierz y znaków*/ char_c < 22)
          {
            //spróbuj odebraæ znak
            c = SER0_GetChar();

            //je¿eli odebrano jakiœ znak to dodaj go do bufora
            if (c != 0)
            {
              response[char_c] = c;
              char_c++;
            }

            //je¿eli odebrany znak to NL (ASCII 10) to zwiêksz licznik linii
            if (c == 10)
              nl_c++;
          }

          //-------------------------------------------------------------------
          //TODO: sprawdziæ w którym znaku kryje siê odpowiedŸ (czy jest to znak na 20 pozycji)
          //-------------------------------------------------------------------
          switch (response[19 /* na takim indeksie jest 22-gi znak*/])
          {
            case '0':
              SER1_PutStr("Terminal nie jest podlaczony do sieci\r\n");
              break;
            case '1':
              SER1_PutStr("Terminal jest podlaczony do sieci\r\n");
              break;
            case '2':
              SER1_PutStr("Terminal szuka stacji bazowej\r\n");
              break;
            case '5':
              SER1_PutStr("Terminal jest w trybie roamingu\r\n");
              break;
            default:
              SER1_PutStr("Nierozpoznany status: ");
              SER1_PutChar(response[19]);
              SER1_PutStr("\r\n");
          }

          //¿eby by³o wiadomo, kiedy koñczy siê dzia³anie komendy i zaczyna przepisywanie odebranych znaków na konsolê
          SER1_PutStr("Koniec\r\n");

          break;

        //znak specjalny ')' powoduje odebranie SMSa
        case ')':
          //podobno jesteœmy user-friendly
          SER1_PutStr("Odbieram sesemesa: \r\n");
          
          //wysy³amy komendê prze³¹czenia siê na tryb tekstowy odbioru SMSa
          SER0_PutStr("AT+CMGF=1\r\n");

          //wysy³amy komendê odbioru SMSa z pozycji 1 w kolejce
          SER0_PutStr("AT+CMGR=1\r\n");

          //-------------------------------------------------------------------
          //Uwaga: w odpowiedzi dostaniemy:
          //-echo komendy AT+CMGF=1
          //-wynik komendy (prawdopodobnie coœ w sytlu "OK")
          //-echo komendy AT+CMGR=1
          //-odpowiedŸ w kilku liniach z dat¹ odebrania SMSa, jego treœci¹ i wynikiem komendy ("OK")
          //razem bêdzie tego chyba z 7 linii
		  //
		  //Przyk³ad:
		  //na UART0 wysy³amy:
		  //AT+CMGF=1<CR><LF>
		  //AT+CMGR=1<CR><LF>
		  //a w odpowiedzi (zgodnie z dokumentacj¹) powinniœmy dostaæ
		  //AT+CMGF=1<CR><LF>
		  //OK<CR><LF>
		  //AT+CMGR=1<CR><LF>
		  //+CMGR:"REC UNREAD","133********",,<CR><LF>
		  //"04/02/25,12 :58 :04+04"<CR><LF>
		  //ABCD<CR><LF>
		  //OK<CR><LF>
          //-------------------------------------------------------------------

          //czyœcimy bufor
          char_c=0;
          nl_c=0;

          //zbieramy ¿niwo
          //-------------------------------------------------------------------
          //TODO: sprawdziæ warunek zakoñczenia zapisywania danych do bufora
          //-------------------------------------------------------------------
          while (/*warunek stopu w oparciu o nl_c lub char_c typu: odbierz x linii lub odbierz y znaków*/ nl_c < 7)
          {
            //spróbuj odebraæ znak
            c = SER0_GetChar();

            //je¿eli odebrano jakiœ znak to dodaj go do bufora
            if (c != 0)
            {
              response[char_c] = c;
              char_c++;
            }

            //-----------------------------------------------------------------
            //TODO: sprawdziæ, czy faktycznie treœæ SMSa znajduje siê w linii 6
            //-----------------------------------------------------------------

            //je¿eli aktualnie odbierasz liniê nr, 6 to przepisz j¹ na konsolê, bo to ona zawiera treœæ SMSa
            if (nl_c == 6)
              SER1_PutChar(c);

            //je¿eli odebrany znak to NL (ASCII 10) to zwiêksz licznik linii
            if (c == 10)
              nl_c++;
          }
          SER1_PutStr("\r\n");

          //¿eby by³o wiadomo, kiedy koñczy siê dzia³anie komendy i zaczyna przepisywanie odebranych znaków na konsolê
          SER1_PutStr("Koniec\r\n");

          break;

        //ka¿dy inny znak przekierowujemy do UART0
        default:
          SER1_PutChar(c);

      } //switch
    } //if
  } //while
} //main()
