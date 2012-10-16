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
 * Jak to dzia�a?
 *
 * 1) Funkcje odbioru znaku z UART0 i UART1 (Serial.c) zosta�y zmienione na nie
 *    blokuj�ce i zwracaj� 0, gdy nie odebrano �adnego znaku
 * 2) Funckja inicjalizacji UARTu (Serial.c) zosta�a tak przerobiona, by
 *    inicjalizowa� oba uarty na raz
 * 3) G��wny program dzia�a tak, �e ka�dy znak z UART0 jest przekierowywany na UART1
 *    a ka�dy znak z UART1 - na UART0. W efekcie mo�na przy u�yciu komputera
 *    komunikwa� si� z modu�em GSM.
 *
 *    Dodatkowo gdy na UART1 (do strony komputera) zostanie wykryty znak specjalny:
 *    '(' lub ')', to zostanie wywo�any odpowiedni algorytm (na p�ytce)
 *
 *    Dla '(' p�ytka spyta modu� o stan sieci i przeka�d odpowied� komputerowi
 *    Dla ')' p�ytka za��da odebrania SMSa z pocz�tku kolejki oczekuj�cych (SMSa nr 1)
 *    i prze�le jego tre�� komputerowi
 */

int main (void) {

  //J: zmienne pomocnicze
  int c;              //znak odebrany przez UART0 lub UART1
  char response[150]; //miejsce na odpowied� modu�u
  int char_c=0;       //liczba znak�w w odpowiedzi
  int nl_c=0;         //liczba linii w odpowiedzi

  SER_Init();                     /* UART0 and UART1 Initialization           */
  
  //J: w p�tli b�dziemy przekazywa� znak po znaku od komputera do modu�u i odwrotnie, chyba �e z komputera dotrze do nas znak specjalny
  while (1) {
    //gdy jaki� znak przyszed� na UART0 (od modu�u)...
    if ((c = SER0_GetChar()) != 0) 
    {
      //je�eli odebrano co� na UART0, to prze�lij to na UART1
      SER1_PutChar(c);
    }

    //gdy jaki� znak przyszed� na UART1 (od komputera)...
    if ((c = SER1_GetChar()) != 0) 
    {
      switch (c)
      {
        //znak specjalny '(' powoduje sprawdzenie stanu sieci
        case '(':
          //podobno jeste�my user-friendly
          SER1_PutStr("Sprawdzam stan zalogowania do sieci: \r\n");
          
          //wysy�amy komend� sprawdzenia stanu sieci
          SER0_PutStr("AT+CREG?\r\n");

          //czy�cimy bufor
          char_c=0;
          nl_c=0;

          //-------------------------------------------------------------------
          //Wydaje mi si�, �e odpowied� na AT+CREG? ma 22 znaki (razem ze znakami
          //nowej linii i echem samej komendy), a cyfra oznaczaj�ca status sieci
          //siedzi w 20 znaku, ale TRZEBA TO SPRAWDZIC NA SPRZ�CIE
		  //
		  //Przyk�ad:
		  //na UART0 leci:
		  //AT+CREG?<CR><LF>
		  //a w odpowiedzi (zgodnie z dokumentacj�) powinni�my dosta�
		  //AT+CREG?<CR><LF>
		  //+CREG: 0,2<CR><LF>
          //-------------------------------------------------------------------

          //zbieramy �niwo
          //-------------------------------------------------------------------
          //TODO: sprawdzi� warunek zako�czenia zapisywania danych do bufora
          //-------------------------------------------------------------------
          while (/*warunek stopu w oparciu o nl_c lub char_c typu: odbierz x linii lub odbierz y znak�w*/ char_c < 22)
          {
            //spr�buj odebra� znak
            c = SER0_GetChar();

            //je�eli odebrano jaki� znak to dodaj go do bufora
            if (c != 0)
            {
              response[char_c] = c;
              char_c++;
            }

            //je�eli odebrany znak to NL (ASCII 10) to zwi�ksz licznik linii
            if (c == 10)
              nl_c++;
          }

          //-------------------------------------------------------------------
          //TODO: sprawdzi� w kt�rym znaku kryje si� odpowied� (czy jest to znak na 20 pozycji)
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

          //�eby by�o wiadomo, kiedy ko�czy si� dzia�anie komendy i zaczyna przepisywanie odebranych znak�w na konsol�
          SER1_PutStr("Koniec\r\n");

          break;

        //znak specjalny ')' powoduje odebranie SMSa
        case ')':
          //podobno jeste�my user-friendly
          SER1_PutStr("Odbieram sesemesa: \r\n");
          
          //wysy�amy komend� prze��czenia si� na tryb tekstowy odbioru SMSa
          SER0_PutStr("AT+CMGF=1\r\n");

          //wysy�amy komend� odbioru SMSa z pozycji 1 w kolejce
          SER0_PutStr("AT+CMGR=1\r\n");

          //-------------------------------------------------------------------
          //Uwaga: w odpowiedzi dostaniemy:
          //-echo komendy AT+CMGF=1
          //-wynik komendy (prawdopodobnie co� w sytlu "OK")
          //-echo komendy AT+CMGR=1
          //-odpowied� w kilku liniach z dat� odebrania SMSa, jego tre�ci� i wynikiem komendy ("OK")
          //razem b�dzie tego chyba z 7 linii
		  //
		  //Przyk�ad:
		  //na UART0 wysy�amy:
		  //AT+CMGF=1<CR><LF>
		  //AT+CMGR=1<CR><LF>
		  //a w odpowiedzi (zgodnie z dokumentacj�) powinni�my dosta�
		  //AT+CMGF=1<CR><LF>
		  //OK<CR><LF>
		  //AT+CMGR=1<CR><LF>
		  //+CMGR:"REC UNREAD","133********",,<CR><LF>
		  //"04/02/25,12 :58 :04+04"<CR><LF>
		  //ABCD<CR><LF>
		  //OK<CR><LF>
          //-------------------------------------------------------------------

          //czy�cimy bufor
          char_c=0;
          nl_c=0;

          //zbieramy �niwo
          //-------------------------------------------------------------------
          //TODO: sprawdzi� warunek zako�czenia zapisywania danych do bufora
          //-------------------------------------------------------------------
          while (/*warunek stopu w oparciu o nl_c lub char_c typu: odbierz x linii lub odbierz y znak�w*/ nl_c < 7)
          {
            //spr�buj odebra� znak
            c = SER0_GetChar();

            //je�eli odebrano jaki� znak to dodaj go do bufora
            if (c != 0)
            {
              response[char_c] = c;
              char_c++;
            }

            //-----------------------------------------------------------------
            //TODO: sprawdzi�, czy faktycznie tre�� SMSa znajduje si� w linii 6
            //-----------------------------------------------------------------

            //je�eli aktualnie odbierasz lini� nr, 6 to przepisz j� na konsol�, bo to ona zawiera tre�� SMSa
            if (nl_c == 6)
              SER1_PutChar(c);

            //je�eli odebrany znak to NL (ASCII 10) to zwi�ksz licznik linii
            if (c == 10)
              nl_c++;
          }
          SER1_PutStr("\r\n");

          //�eby by�o wiadomo, kiedy ko�czy si� dzia�anie komendy i zaczyna przepisywanie odebranych znak�w na konsol�
          SER1_PutStr("Koniec\r\n");

          break;

        //ka�dy inny znak przekierowujemy do UART0
        default:
          SER1_PutChar(c);

      } //switch
    } //if
  } //while
} //main()
