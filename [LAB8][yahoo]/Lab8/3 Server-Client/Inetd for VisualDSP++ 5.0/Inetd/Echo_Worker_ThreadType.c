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
  
  //poniewa� recv potrafi zwraca� po jednym bajcie, tutaj b�dziemy zbiera� dane, kt�re przysz�y do momentu a� nie przyjdzie znak nowej linii (=koniec komendy)
  #define COMMAND_BUFFER_SIZE 1024 //ilosc pami�ci zaalokowanej na nasz bufor
  char command_buffer[COMMAND_BUFFER_SIZE]; //tu b�dziemy zbiera� do kupy to co przyjdzie sieci�
  char command_buffer_length=0; //tu bedziemy zlicza� ile znak�w ju� przysz�o
  int i;
   
  //g��wna p�tla programu
  while (1)
  {
    int r; //wynik dzia�ania funkcji biblioteki BSD Sockets (najcz�ciej liczba odebranych/wys�anych bajt�w)
 
    //odbieramy dane z podanego nam przez proces serwera socketu do bufora
    r = recv(p->socket, p->buff, p->buff_len /* niby ��damy tyle danych ile jest miejsca w buforze, ale skurwiel potrafi odbiera� nawet po jednym bajcie! */, 0 /* bez dodatkowych flag */);
    
    //je�eli operacja odbioru zako�czy�a si� b��dem, przerywamy p�tl� i zamykamy proces workera
    if (r <= 0)
      break;

    //je�eli cos odebrano to trzeba to przetworzy�
    if (r > 0)
    {
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
        //poniewaz poprawna komenda ma 4 znaki ("ping") to na pi�tej pozycji w buforze wpisujemy znak konca ciagu i por�wnujemy bufor ze wzorcem
        if (command_buffer_length>4)
        {
          command_buffer[4] = 0; //znak ko�ca ci�gu
          if (strcmp(command_buffer, "ping")==0)
          {
            //odebrany ci�g to "ping", odsy�amy wi�c ci�g "pong"
            strcpy(p->buff, "pong\n");
            r = send(p->socket, p->buff, 6 /* "pong" + znak nowej linii + znak ko�ca ci�gu */, 0 /* bez dodatkowych flag */);

            //je�eli podczas nadawania wyst�pi� b��d, to wychodzimy z p�tli  
            if (r <= 0)
              break;
          }
        }
        
        //bedziemy chcieli zapelniac bufor z komenda od nowa
        command_buffer_length = 0;
      }
    }
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
