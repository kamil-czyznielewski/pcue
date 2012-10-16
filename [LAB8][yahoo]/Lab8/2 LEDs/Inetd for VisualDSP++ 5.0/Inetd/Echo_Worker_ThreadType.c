/* =============================================================================
 *
 *  Description: This is a C implementation for Thread Echo_Worker_ThreadType
 *
 * -----------------------------------------------------------------------------
 *  Comments:
 *
 * ===========================================================================*/

/* Get access to any of the VDK features & datatypes used */
#include "Echo_Worker_ThreadType.h"
#include "charcb.h"
#include "IPv4_util.h"
#include <cdefBF537.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>

void
Echo_Worker_ThreadType_RunFunction(void **inPtr)
{
  /* Put the thread's "main" Initialization HERE */
  charcb_t* p = *(charcb_t**)inPtr;

  /*
   * budowa struktury charcb_t (za: charcb.h)
   * 
   * typedef struct _charcb 
   * {
   *    int    socket;
   *    struct sockaddr_in cliaddr;
   *    int    clilen;
   *    int    nextchar;
   *    char*  buff;
   *    int    buff_len;
   * } charcb_t;
   */

  /*
   * To od czego by tu zacz��...?
   * 
   * W tym pliku zawarty jest kod procesu-workera dla us�ugi echo. Proces serwera po nawi�zaniu po��czenia z klientem
   * tworzy proces pracownika i przekazuje mu funkcj� robienia tego co potrzebne z takim, ju� nawi�zanym po��czeniem.
   * Do tej funkcji dane po��czenia (socket i inne pierdo�y) trafiaj� w agrumencie inPtr. Rol� workera us�ugi echo jest
   * odebranie danych z po��czenia i wys�anie ich bez �adnej modyfikacji tym samym po��czeniem do klienta.
   *
   * My dodatkowo w ramach us�ugi echo sprawdzimy czy odebrany tekst nie jest czasem pewn� komend� steruj�c�, a je�eli jest,
   * zapalimy lub zgasimy jedn� z diod (di�d?) LED na p�ytce.
   */
  
  /*
   * Diody...
   *
   * Diody na p�ytce EZ-KIT s� pod��czone do portu wejscia/wyjscia PORTF. Manual podpowiada, �e s� to piny
   * 6,7,8,9,10,11 (licz�c od 0), por. ADSP_BF537_EZ_KIT_Lite_Manual_Rev._2.2.pdf, str 85. Dodatkowo manual rzecze, �e ka�da dioda
   * z jednej strony pod��czona jest (przez jakis tam ssmieszny multiplekser) do procesora, a z drugiej do masy. W skr�cie oznacza
   * to, �e do pinu portu skojarzonego z diod� musimy wpisa� 1, �eby j� zapali�, a 0, by j� zgasi�.
   */
   
  /*
   * PORTF
   *
   * Ca�a zabawa polega na tym, �e jest wyj�tkowo du�o rejestr�w steruj�cych portem PORTF. W programie mo�na si� do nich dobra� poprzez
   * *p[nazwa_rejestru], np. *pPORTF_IO = 0x0000;
   * 
   * Rejestry jakie mamy (za: BF537_HRM_whole_book_o.pdf, str 1102, 864):
   * PORTFIO - rejestr danych (chyba do odczytu i zapisu)
   * PORTF_FER - function enable register. Wi�kszo� pin�w portu ma tak�e dodatkow� funkcj� (pin interfejcu CAN, UART, etc.).
   *             Ustawienie 0 dla pewnego pinu w tym rejestrze sprawia, �e dodatkowa funkcjonalnoc b�dzie wy��czona i pin
   *             b�dzie s�u�y� tylko jako zwyk�y pin wejcia/wyjcia
   * PORTFIO_DIR - kierunek dzia�ania ka�dego pinu (0-wejcie, 1-wyjscie)
   * PORTFIO_INEN - input enable register (?, chyba s�u�y do w��czania przepisywania wartoci z pinu do bitu rejestru PORTFIO)
   * PORTFIO_SET - fajerwerk, uproszczenie korzytania z PORTFIO: zapisz tam 1, by ustawi� dany pin; 0 nic nie zmienia
   * PORTFIO_CLEAR - jak wy�ej, zapisz 1 w odpowiednie miejsce by wyzerowa� dany pin; 0 nic nie zmienia
   * PORTFIO_TROGGLE - jak wy�ej, zapisz 1 w odpowiednie miejsce by prze��czy� stan pinu na przeciwny; 0 nic nie zmienia
   *
   * (pozosta�e rejestry, w tym te zwi�zane z generowaniem przerwa� nas teraz nie interesuj�)
   *
   * Aha, nie napisa�em jeszcze, �e PORTF jest portem 16-pinowym
   */
   
  //nic nie stoi na przeszkodzie, by przy ka�dym uruchomieniu procesu workera (a wi�c przy ka�dym nowym po��czeniu) od nowa konfigurowa� port PORTF
  *pPORTFIO_INEN  = 0x0000; //=0b0000000000000000; �aden pin nie ma mie� w��czonej funkcji wejcia
  *pPORTFIO_DIR   = 0x0FC0; //=0b0000111111000000; kierunki dzia�ania pin�w (wyjcia maj� by� tam gdzie s� diody)
  *pPORTFIO_SET   = 0x0FC0; //zapalamy wszystkie diody
  *pPORTFIO_CLEAR = 0x0FC0; //gasimy wszystkie diody
  
  /* i teraz mo�emy si� nimi bawi� do woli */

  //poniewa� recv potrafi zwraca� po jednym bajcie, tutaj b�dziemy zbiera� dane, kt�re przysz�y do momentu a� nie przyjdzie znak nowej linii (=koniec komendy)
  #define COMMAND_BUFFER_SIZE 1024 //ilosc pami�ci zaalokowanej na nasz bufor
  char command_buffer[COMMAND_BUFFER_SIZE]; //tu b�dziemy zbiera� do kupy to co przyjdzie sieci�
  char command_buffer_length=0; //tu bedziemy zlicza� ile znak�w ju� przysz�o
  int i;
  unsigned int led_no; 
  
  //g��wna p�tla programu
  while (1)
  {
    int r; //wynik dzia�ania funkcji biblioteki BSD Sockets (najcz�ciej liczba odebranych/wys�anych bajt�w)
 
    //odbieramy dane z podanego nam przez proces serwera socketu do bufora
    r = recv(p->socket, p->buff, p->buff_len /* niby ��damy tyle danych ile jest miejsca w buforze, ale skurwiel potrafi odbiera� nawet po jednym bajcie! */, 0 /* bez dodatkowych flag */);
    
    //je�eli operacja odbioru zako�czy�a si� b��dem, przerywamy p�tl� i zamykamy proces workera
    if (r <= 0)
      break;

    //je�eli cos odebrano to trzeba to przetworzy�: odsy�amy dane do nadawcy
    if (r > 0)
    {
      /*
       * komendy, na kt�re reagujemy to:
       * sx - set diode x (x=0..5)
       * rx - reset diode x
       * tx - troggle diode x
       */
       
      //przede wszystkim bysmy musieli dopisac to co odebralismy do naszego bufora (command_buffer)
      for (i=0; (i<r/* nie przepisalismy jeszcze wszystkich znakow */) && (command_buffer_length<COMMAND_BUFFER_SIZE /* nie skonczyl nam sie bufor */); i++)
      {
        //dopisujemy kolejny odebrany znak do bufora
        command_buffer[command_buffer_length] = p->buff[i];
        command_buffer_length++;
      }
       
      //bufor moze byc uciety, wiec sprawdzamy NA ODEBRANYCH DANYCH czy ostatni znak nie byl czasem znakiem nowej linii (wtedy zawartsc bufora nalezy potraktowac jako calkowicie odebrana komende)
      if (p->buff[r-1]=='\n')
      {
        //bedziemy chcieli zapelniac bufor z komenda od nowa
        command_buffer_length = 0;

        //analizujemy to co do tej pory zebralismy - ustalamy numer diody (powinien byc zapisany w ascii na drugim bajcie komendy)
        led_no = command_buffer[1];
           
        //t�umaczymy numer diody z ascii na inta (por. tablica znak�w ASCII)
        led_no -= 48;
           
        //sprawdzamy czy pierwszy bajt bufora to znak oznaczajacy poprawna komende
        switch(command_buffer[0])
        {
          case 's':
            //zapisujemy jedynk� na pozycji odpowiadajacej diodzie w odpowiednim rejestrze
            *pPORTFIO_SET = 0x0001 << (led_no + 5 /* bo dioda 0 jest na pinie 5, dioda 1 na 6, etc. */);
            break;

          case 'r':
            //zapisujemy jedynk� na pozycji odpowiadajacej diodzie w odpowiednim rejestrze
            *pPORTFIO_CLEAR = 0x0001 << (led_no + 5 /* bo dioda 0 jest na pinie 5, dioda 1 na 6, etc. */);
            break;

          case 't':
            //zapisujemy jedynk� na pozycji odpowiadajacej diodzie w odpowiednim rejestrze
            //nie wiedzie� czemu gdzies brakuje definicji pPORTFIO_TROGGLE
            //*pPORTFIO_TROGGLE = 0x0001 << (led_no + 5 /* bo dioda 0 jest na pinie 5, dioda 1 na 6, etc. */);
            break;
        }
      }

      //odsy�amy to co odebralismy
      r = send(p->socket, p->buff, r /* odsy�amy dok�adnie tyle danych ile odebralismy */, 0 /* bez dodatkowych flag */);
    }
    
    //je�eli dla odmiany podczas nadawania wyst�pi� b��d, to wychodzimy z p�tli  
    if (r <= 0)
      break;
    
    //zrzekamy si� czasu procesora tak, �eby inne w�tki te� mog�y si� troch� powykonywa�
    VDK_Yield();
  }

  close(p->socket);
  printf("   Echo connection from IP %s Port %u - closed\n", 
          IPv4_straddr(ntohl(p->cliaddr.sin_addr.s_addr)),
          ntohs(p->cliaddr.sin_port));
  p->socket = -1;

    /* Put the thread's exit from "main" HERE */
    /* A thread is automatically Destroyed when it exits its run function */
}

int
Echo_Worker_ThreadType_ErrorFunction(void **inPtr)
{

    /* TODO - Put this thread's error handling code HERE */

      /* The default ErrorHandler kills the thread */

	VDK_DestroyThread(VDK_GetThreadID(), false);
	return 0;
}

void
Echo_Worker_ThreadType_InitFunction(void **inPtr, VDK_ThreadCreationBlock *pTCB)
{
    /* Put code to be executed when this thread has just been created HERE */

    /* This routine does NOT run in new thread's context.  Any non-static thread
     *   initialization should be performed at the beginning of "Run()."
     */
}

void
Echo_Worker_ThreadType_DestroyFunction(void **inPtr)
{
    /* Put code to be executed when this thread is destroyed HERE */

    /* This routine does NOT run in the thread's context.  Any VDK API calls
     *   should be performed at the end of "Run()."
     */
}

/* ========================================================================== */
