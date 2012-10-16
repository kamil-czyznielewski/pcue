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
   * To od czego by tu zacz¹æ...?
   * 
   * W tym pliku zawarty jest kod procesu-workera dla us³ugi echo. Proces serwera po nawi¹zaniu po³¹czenia z klientem
   * tworzy proces pracownika i przekazuje mu funkcjê robienia tego co potrzebne z takim, ju¿ nawi¹zanym po³¹czeniem.
   * Do tej funkcji dane po³¹czenia (socket i inne pierdo³y) trafiaj¹ w agrumencie inPtr. Rol¹ workera us³ugi echo jest
   * odebranie danych z po³¹czenia i wys³anie ich bez ¿adnej modyfikacji tym samym po³¹czeniem do klienta.
   *
   * My dodatkowo w ramach us³ugi echo sprawdzimy czy odebrany tekst nie jest czasem pewn¹ komend¹ steruj¹c¹, a je¿eli jest,
   * zapalimy lub zgasimy jedn¹ z diod (diód?) LED na p³ytce.
   */
  
  /*
   * Diody...
   *
   * Diody na p³ytce EZ-KIT s¹ pod³¹czone do portu wejscia/wyjscia PORTF. Manual podpowiada, ¿e s¹ to piny
   * 6,7,8,9,10,11 (licz¹c od 0), por. ADSP_BF537_EZ_KIT_Lite_Manual_Rev._2.2.pdf, str 85. Dodatkowo manual rzecze, ¿e ka¿da dioda
   * z jednej strony pod³¹czona jest (przez jakis tam ssmieszny multiplekser) do procesora, a z drugiej do masy. W skrócie oznacza
   * to, ¿e do pinu portu skojarzonego z diod¹ musimy wpisaæ 1, ¿eby j¹ zapaliæ, a 0, by j¹ zgasiæ.
   */
   
  /*
   * PORTF
   *
   * Ca³a zabawa polega na tym, ¿e jest wyj¹tkowo du¿o rejestrów steruj¹cych portem PORTF. W programie mo¿na siê do nich dobraæ poprzez
   * *p[nazwa_rejestru], np. *pPORTF_IO = 0x0000;
   * 
   * Rejestry jakie mamy (za: BF537_HRM_whole_book_o.pdf, str 1102, 864):
   * PORTFIO - rejestr danych (chyba do odczytu i zapisu)
   * PORTF_FER - function enable register. Wiêkszoæ pinów portu ma tak¿e dodatkow¹ funkcjê (pin interfejcu CAN, UART, etc.).
   *             Ustawienie 0 dla pewnego pinu w tym rejestrze sprawia, ¿e dodatkowa funkcjonalnoc bêdzie wy³¹czona i pin
   *             bêdzie s³u¿y³ tylko jako zwyk³y pin wejcia/wyjcia
   * PORTFIO_DIR - kierunek dzia³ania ka¿dego pinu (0-wejcie, 1-wyjscie)
   * PORTFIO_INEN - input enable register (?, chyba s³u¿y do w³¹czania przepisywania wartoci z pinu do bitu rejestru PORTFIO)
   * PORTFIO_SET - fajerwerk, uproszczenie korzytania z PORTFIO: zapisz tam 1, by ustawiæ dany pin; 0 nic nie zmienia
   * PORTFIO_CLEAR - jak wy¿ej, zapisz 1 w odpowiednie miejsce by wyzerowaæ dany pin; 0 nic nie zmienia
   * PORTFIO_TROGGLE - jak wy¿ej, zapisz 1 w odpowiednie miejsce by prze³¹czyæ stan pinu na przeciwny; 0 nic nie zmienia
   *
   * (pozosta³e rejestry, w tym te zwi¹zane z generowaniem przerwañ nas teraz nie interesuj¹)
   *
   * Aha, nie napisa³em jeszcze, ¿e PORTF jest portem 16-pinowym
   */
   
  //nic nie stoi na przeszkodzie, by przy ka¿dym uruchomieniu procesu workera (a wiêc przy ka¿dym nowym po³¹czeniu) od nowa konfigurowaæ port PORTF
  *pPORTFIO_INEN  = 0x0000; //=0b0000000000000000; ¿aden pin nie ma mieæ w³¹czonej funkcji wejcia
  *pPORTFIO_DIR   = 0x0FC0; //=0b0000111111000000; kierunki dzia³ania pinów (wyjcia maj¹ byæ tam gdzie s¹ diody)
  *pPORTFIO_SET   = 0x0FC0; //zapalamy wszystkie diody
  *pPORTFIO_CLEAR = 0x0FC0; //gasimy wszystkie diody
  
  /* i teraz mo¿emy siê nimi bawiæ do woli */

  //poniewa¿ recv potrafi zwracaæ po jednym bajcie, tutaj bêdziemy zbieraæ dane, które przysz³y do momentu a¿ nie przyjdzie znak nowej linii (=koniec komendy)
  #define COMMAND_BUFFER_SIZE 1024 //ilosc pamiêci zaalokowanej na nasz bufor
  char command_buffer[COMMAND_BUFFER_SIZE]; //tu bêdziemy zbieraæ do kupy to co przyjdzie sieci¹
  char command_buffer_length=0; //tu bedziemy zliczaæ ile znaków ju¿ przysz³o
  int i;
  unsigned int led_no; 
  
  //g³ówna pêtla programu
  while (1)
  {
    int r; //wynik dzia³ania funkcji biblioteki BSD Sockets (najczêciej liczba odebranych/wys³anych bajtów)
 
    //odbieramy dane z podanego nam przez proces serwera socketu do bufora
    r = recv(p->socket, p->buff, p->buff_len /* niby ¿¹damy tyle danych ile jest miejsca w buforze, ale skurwiel potrafi odbieraæ nawet po jednym bajcie! */, 0 /* bez dodatkowych flag */);
    
    //je¿eli operacja odbioru zakoñczy³a siê b³êdem, przerywamy pêtlê i zamykamy proces workera
    if (r <= 0)
      break;

    //je¿eli cos odebrano to trzeba to przetworzyæ: odsy³amy dane do nadawcy
    if (r > 0)
    {
      /*
       * komendy, na które reagujemy to:
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
           
        //t³umaczymy numer diody z ascii na inta (por. tablica znaków ASCII)
        led_no -= 48;
           
        //sprawdzamy czy pierwszy bajt bufora to znak oznaczajacy poprawna komende
        switch(command_buffer[0])
        {
          case 's':
            //zapisujemy jedynkê na pozycji odpowiadajacej diodzie w odpowiednim rejestrze
            *pPORTFIO_SET = 0x0001 << (led_no + 5 /* bo dioda 0 jest na pinie 5, dioda 1 na 6, etc. */);
            break;

          case 'r':
            //zapisujemy jedynkê na pozycji odpowiadajacej diodzie w odpowiednim rejestrze
            *pPORTFIO_CLEAR = 0x0001 << (led_no + 5 /* bo dioda 0 jest na pinie 5, dioda 1 na 6, etc. */);
            break;

          case 't':
            //zapisujemy jedynkê na pozycji odpowiadajacej diodzie w odpowiednim rejestrze
            //nie wiedzieæ czemu gdzies brakuje definicji pPORTFIO_TROGGLE
            //*pPORTFIO_TROGGLE = 0x0001 << (led_no + 5 /* bo dioda 0 jest na pinie 5, dioda 1 na 6, etc. */);
            break;
        }
      }

      //odsy³amy to co odebralismy
      r = send(p->socket, p->buff, r /* odsy³amy dok³adnie tyle danych ile odebralismy */, 0 /* bez dodatkowych flag */);
    }
    
    //je¿eli dla odmiany podczas nadawania wyst¹pi³ b³¹d, to wychodzimy z pêtli  
    if (r <= 0)
      break;
    
    //zrzekamy siê czasu procesora tak, ¿eby inne w¹tki te¿ mog³y siê trochê powykonywaæ
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
