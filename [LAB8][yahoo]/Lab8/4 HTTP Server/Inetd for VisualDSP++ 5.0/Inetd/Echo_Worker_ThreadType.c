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
   * Przyk³adowe ¿¹danie i odpowiedŸ HTTP GET
   *
   * GET /path/file.html HTTP/1.0
   * From: someuser@jmarshall.com
   * User-Agent: HTTPTool/1.0
   * [pusta linia]
   *
   *
   *
   * HTTP/1.0 200 OK
   * Date: Fri, 31 Dec 1999 23:59:59 GMT
   * Content-Type: text/html
   * Content-Length: 48
   * [pusta linia]
   * <html>
   * <body>
   * <h1>Hello World!</h1>
   * </body>
   * </html>
   */
   
  //bufor do przechowywania ¿¹dania HTTP
  #define REQUEST_SIZE 1500 //tyle mniej wiêcej maksymalnie bajtów mo¿e mieæ pakiet TCP
  char request_buffer[REQUEST_SIZE]; //tu bêdziemy przechowywaæ kopiê zapytania
  char *response_200 = "HTTP/1.0 200 OK\n"
                       "Date: Fri, 31 Dec 1999 23:59:59 GMT\n"
                       "Content-Type: text/html\n"
                       "Content-Length: 63\n\n"
                       "<html>OK <a href=\"javascript:history.back()\">wstecz</a></html>\n"; //odpowiedŸ HTTP 200 OK
  char *response_404 = "HTTP/1.0 404 Not Found\n"
                       "Date: Fri, 31 Dec 1999 23:59:59 GMT\n"
                       "Content-Type: text/plain\n"
                       "Content-Length: 15\n\n"
                       "Nie znaleziono\n"; //odpowiedŸ HTTP 404 Not Found
  char *response_200i = "HTTP/1.0 200 OK\n"
                        "Date: Fri, 31 Dec 1999 23:59:59 GMT\n"
                        "Content-Type: text/html\n"
                        "Content-Length: 344\n\n"
                        "<html>"
                        "<a href=\"\/s0\">set 0</a> | <a href=\"\/r0\">reset 0</a><br>"
                        "<a href=\"\/s1\">set 1</a> | <a href=\"\/r1\">reset 1</a><br>"
                        "<a href=\"\/s2\">set 2</a> | <a href=\"\/r2\">reset 2</a><br>"
                        "<a href=\"\/s3\">set 3</a> | <a href=\"\/r3\">reset 3</a><br>"
                        "<a href=\"\/s4\">set 4</a> | <a href=\"\/r4\">reset 4</a><br>"
                        "<a href=\"\/s5\">set 5</a> | <a href=\"\/r5\">reset 5</a><br>"
                        "</html>\n"; //pocz¹tkowa odpowiedŸ HTTP 200 OK (z menu)
   
  int r; //wynik dzia³ania funkcji biblioteki BSD Sockets (najczêciej liczba odebranych/wys³anych bajtów)
  char led; //numer diody, na której bêdziemy wykonywaæ operacje
 
  /*
   * Do tej pory zak³ada³em, ¿e funkcja recv dzia³a z³osliwie zwracaj¹c zawsze (a przynajmniej czêsto)
   * po jednym bajcie z odebranego ci¹gu. Okazuje siê jednak, ¿e to klient us³ugi echo nadawa³ po
   * jednym bajcie, zatem tu za³o¿ymy sobie, ¿e zawsze ¿¹danie HTTP przychodzi w ca³osci.
   */
  r = recv(p->socket, request_buffer, REQUEST_SIZE, 0 /* bez dodatkowych flag */);
    
  //je¿eli cos odebrano to trzeba to przetworzyæ
  if (r > 0)
  {
    if (request_buffer[5]=='s') //5-ty znak w ¿¹daniu to pierwsza litera po "/" w nazwie ¿¹danego pliku (obs³ugujemy wartoœci "s" - set diode i "r" - reset diode)
    {
      led = request_buffer[6] - 48; //konwertujemy znak ascii na cyfrê
//      printf("SET: %d\r\n", led);
      led += 6; //dioda 0 jest na 6 pinie portu PORTFIO

      //zapalamy diodê
      *pPORTFIO_SET = 1 << led; 

      //wysy³amy odpowiedŸ
      r = send(p->socket, response_200, strlen(response_200), 0 /* bez dodatkowych flag */);

    } else if (request_buffer[5]=='r') {
      led = request_buffer[6] - 48; //konwertujemy znak ascii na cyfrê
//      printf("RESET: %d\r\n", led);
      led += 6; //dioda 0 jest na 6 pinie portu PORTFIO

      //gasimy diodê
      *pPORTFIO_CLEAR = 1 << led;

      //wysy³amy odpowiedŸ
      r = send(p->socket, response_200, strlen(response_200), 0 /* bez dodatkowych flag */);

    } else { //a je¿eli zapytanie nie pasuje do wzorca, to zamiast HTTP 404 wysy³amy indeks
      r = send(p->socket, response_200i, strlen(response_200i), 0 /* bez dodatkowych flag */);        
    }
  }

  //ze wzglêdu na mechanizm keep-alive fajnie, gdyby to serwer zamyka³ po³¹czenie po wys³aniu strony
  close(p->socket);
//  printf("   Echo connection from IP %s Port %u - closed\n", 
//          IPv4_straddr(ntohl(p->cliaddr.sin_addr.s_addr)),
//          ntohs(p->cliaddr.sin_port));
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
