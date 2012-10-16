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
  
  while (1)
  {
    int r;
 
    r = recv(p->socket, p->buff, p->buff_len, 0);
    if (r <= 0)
      break;

    if (r > 0)
      r = send(p->socket, p->buff, r, 0);
    if (r <= 0)
      break;
      
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
