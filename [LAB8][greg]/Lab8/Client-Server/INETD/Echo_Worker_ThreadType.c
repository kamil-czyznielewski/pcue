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

#include <lwip/sockets.h>
#include <cdefBF537.h>
#include <lwip/inet.h>

void Init_Flags(void)
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
}

void Echo_Worker_ThreadType_RunFunction(void **inPtr)
{
	int r;
	char path[100]; 
	int LED;
	
  	//code  
  	charcb_t* p = *(charcb_t**)inPtr;
  	Init_Flags();
  	*pPORTFIO_CLEAR = 0x0FC4;
      
    
    while(1) 
    {
	    r = recv(p->socket, p->buff, p->buff_len, 0);
	    sscanf(p->buff, "%s", path);     	
	      
    	
	    if(path[0] == '1')
	    {
	    	*pPORTFIO_SET = 0x0040 | 0x0080 | 0x0100 | 0x0200 | 0x0400 | 0x0800;
	    	strcpy(p->buff, "up.");
	    	r = 4;
	    } else if(path[0] == '2')
	    {
	    	*pPORTFIO_CLEAR = 0x0FC4;
	    	strcpy(p->buff, "down.");
	    	r = 6;
	    } else {
	    	strcpy(p->buff, "");
	    	r = 1;
	    }
    
	    //if (r <= 0)
	      //break;
    
	    if (r > 0)
	    {
	      r = send(p->socket, p->buff, r, 0);
	    }
	    if (r <= 0)
	      break;
      
      	VDK_Yield();
    }
      
      
  close(p->socket);
  printf("   XX connection from IP %s Port %u - closed\n", 
          IPv4_straddr(ntohl(p->cliaddr.sin_addr.s_addr)),
          ntohs(p->cliaddr.sin_port));
  p->socket = -1;

    /* Put the thread's exit from "main" HERE */
    /* A thread is automatically Destroyed when it exits its run function */
}








int Echo_Worker_ThreadType_ErrorFunction(void **inPtr)
{

    /* TODO - Put this thread's error handling code HERE */

      /* The default ErrorHandler kills the thread */

	VDK_DestroyThread(VDK_GetThreadID(), false);
	return 0;
}

void Echo_Worker_ThreadType_InitFunction(void **inPtr, VDK_ThreadCreationBlock *pTCB)
{
    /* Put code to be executed when this thread has just been created HERE */

    /* This routine does NOT run in new thread's context.  Any non-static thread
     *   initialization should be performed at the beginning of "Run()."
     */
}

void Echo_Worker_ThreadType_DestroyFunction(void **inPtr)
{
    /* Put code to be executed when this thread is destroyed HERE */

    /* This routine does NOT run in the thread's context.  Any VDK API calls
     *   should be performed at the end of "Run()."
     */
}

/* ========================================================================== */

