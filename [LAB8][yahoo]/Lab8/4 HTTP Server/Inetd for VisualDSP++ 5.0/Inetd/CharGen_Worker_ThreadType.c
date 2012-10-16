/* =============================================================================
 *
 *  Description: This is a C implementation for Thread CharGen_Worker_ThreadType
 *
 * -----------------------------------------------------------------------------
 *  Comments:
 *
 * ===========================================================================*/

/* Get access to any of the VDK features & datatypes used */
#include "CharGen_Worker_ThreadType.h"
#include "charcb.h"
#include "IPv4_util.h"

#include <lwip/sockets.h>
#include <lwip/inet.h>

#define MAX_WORKERS 3
#define MAX_CHARS  72

void
CharGen_Worker_ThreadType_RunFunction(void **inPtr)
{
    /* Put the thread's "main" Initialization HERE */
  charcb_t* p = *(charcb_t**)inPtr;
  const int MAX_LINES = p->buff_len / (MAX_CHARS + 2);
  char (*lines)[][MAX_CHARS + 2] = (void*)p->buff;
  
  
  p->nextchar = 0x21;
  
  while (1) {
    int i, j, k;
    int x, r;

    // create data to be sent - lines of (72 chars + CR,LF)
    for (i = 0; i < MAX_LINES; i += 1) {
      int ch = p->nextchar;
      
      for (j = 0; j < MAX_CHARS; j += 1) {
        (*lines)[i][j] = ch++;
        if (ch == 0x7F)
          ch = 0x21;
      }

      (*lines)[i][MAX_CHARS]     = '\r';
      (*lines)[i][MAX_CHARS + 1] = '\n';

      p->nextchar += 1;
      if (p->nextchar == 0x7F)
        p->nextchar = 0x21;
    }
    
    r = send(p->socket, p->buff, MAX_LINES * (MAX_CHARS + 2), 0);
    if (r <= 0)
      break;
    
    VDK_Yield();
  }
  close(p->socket);
  printf("CharGen connection from IP %s Port %u - closed\n", 
          IPv4_straddr(ntohl(p->cliaddr.sin_addr.s_addr)),
          ntohs(p->cliaddr.sin_port));
  p->socket = -1;
    /* Put the thread's exit from "main" HERE */
    /* A thread is automatically Destroyed when it exits its run function */
}

int
CharGen_Worker_ThreadType_ErrorFunction(void **inPtr)
{

    /* TODO - Put this thread's error handling code HERE */

      /* The default ErrorHandler kills the thread */

	VDK_DestroyThread(VDK_GetThreadID(), false);
	return 0;
}

void
CharGen_Worker_ThreadType_InitFunction(void **inPtr, VDK_ThreadCreationBlock *pTCB)
{
    /* Put code to be executed when this thread has just been created HERE */

    /* This routine does NOT run in new thread's context.  Any non-static thread
     *   initialization should be performed at the beginning of "Run()."
     */
}

void
CharGen_Worker_ThreadType_DestroyFunction(void **inPtr)
{
    /* Put code to be executed when this thread is destroyed HERE */

    /* This routine does NOT run in the thread's context.  Any VDK API calls
     *   should be performed at the end of "Run()."
     */
}

/* ========================================================================== */
