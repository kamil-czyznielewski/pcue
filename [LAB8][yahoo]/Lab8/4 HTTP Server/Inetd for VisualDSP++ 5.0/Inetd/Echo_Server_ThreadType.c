/* =============================================================================
 *
 *  Description: This is a C implementation for Thread Echo_Server_ThreadType
 *
 * -----------------------------------------------------------------------------
 *  Comments:
 *
 * ===========================================================================*/

/* Get access to any of the VDK features & datatypes used */
#include "Echo_Server_ThreadType.h"
#include "charcb.h"
#include "IPv4_util.h"

#include <stddef.h>
#include <cdefBF537.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>

#define MAX_WORKERS 3
charcb_t ebs[MAX_WORKERS];

void
Echo_Server_ThreadType_RunFunction(void **inPtr)
{
    /* Put the thread's "main" Initialization HERE */
  int listenfd;
  struct sockaddr_in saddr;
  int c;
  char *echo_area_start=NULL;
  
  echo_area_start =(char*)malloc(MAX_WORKERS * sizeof(char) * ECHO_BUFFER_SIZE);
  if(echo_area_start == NULL)
  {
  	printf("failed to allocate memory to discard server\n");
  	return;
  }
  
  printf("   Echo Server running (port 7)\n");

  // First acquire our socket for listening for connections
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    fprintf(stderr, "Echo Server: socket create failed\n");
    exit(300);
  }
  
  // set up stream info
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = htonl(INADDR_ANY);
  saddr.sin_port = htons(80);     // Http service well-known port number

  if (bind(listenfd, (struct sockaddr*)&saddr, sizeof(saddr)) == -1) {
    fprintf(stderr, "Echo Server: socket bind failed");
    exit(301);
  }

  // Put socket into listening mode
  if (listen(listenfd, MAX_WORKERS) == -1) {
    fprintf(stderr, "Echo Server: listen failed");
    exit(302);
  }
  
  // mark all ebs entries free and set up buffer addresses and lengths
  for (c = 0; c < MAX_WORKERS; c += 1) {
    ebs[c].socket = -1;
    ebs[c].buff = (char*)(echo_area_start + (c * ECHO_BUFFER_SIZE));
    ebs[c].buff_len = ECHO_BUFFER_SIZE;
  }
  
  // loop forever awaiting connections on port 80
  while (1)
  {
    charcb_t* p;
    charcb_t  eb;
    
    memset(&eb, 0, sizeof(charcb_t));
    eb.clilen = sizeof(struct sockaddr);
    eb.socket = accept(listenfd, (struct sockaddr*)&eb.cliaddr, &eb.clilen);
    
    if (eb.socket == -1) {
      fprintf(stderr, "Echo Server: accept() error\n");
      exit(303);
    } else {
      struct VDK_ThreadCreationBlock TCB = {
        kEcho_Worker_ThreadType,
        (VDK_ThreadID)0,
        0,
        (VDK_Priority)0
      };
        
//      printf("   Echo connection from IP %s Port %u - ", 
//              IPv4_straddr(ntohl(eb.cliaddr.sin_addr.s_addr)),
//              ntohs(eb.cliaddr.sin_port));
    
      // find a free eb
      for (c = 0; c < MAX_WORKERS; c += 1) {
        if (ebs[c].socket == -1) {
          p = &ebs[c];
          memcpy(p, &eb, offsetof(charcb_t, nextchar));
          break;
        }
      }
      
      if (c == MAX_WORKERS) {
        // too many live connections - just close this one
        close(eb.socket);
        printf("dropped\n");
      } else {
//        printf("accepted\n");
        TCB.user_data_ptr = (void*)p;
        if (VDK_CreateThreadEx(&TCB) == UINT_MAX) {
          fprintf(stderr, "Echo Server: failed to create worker thread\n");
          exit(305);
        }
      }
    }
  }

    /* Put the thread's exit from "main" HERE */
    /* A thread is automatically Destroyed when it exits its run function */
}

int
Echo_Server_ThreadType_ErrorFunction(void **inPtr)
{

    /* TODO - Put this thread's error handling code HERE */

      /* The default ErrorHandler kills the thread */

	VDK_DestroyThread(VDK_GetThreadID(), false);
	return 0;
}

void
Echo_Server_ThreadType_InitFunction(void **inPtr, VDK_ThreadCreationBlock *pTCB)
{
    /* Put code to be executed when this thread has just been created HERE */

    /* This routine does NOT run in new thread's context.  Any non-static thread
     *   initialization should be performed at the beginning of "Run()."
     */
     
    *pPORTFIO_INEN  = 0x0000; //=0b0000000000000000; ¿aden pin nie ma mieæ w³¹czonej funkcji wejcia
    *pPORTFIO_DIR   = 0x0FC0; //=0b0000111111000000; kierunki dzia³ania pinów (wyjcia maj¹ byæ tam gdzie s¹ diody)
    *pPORTFIO_SET   = 0x0FC0;
    *pPORTFIO_CLEAR = 0x0FC0;
}

void
Echo_Server_ThreadType_DestroyFunction(void **inPtr)
{
    /* Put code to be executed when this thread is destroyed HERE */

    /* This routine does NOT run in the thread's context.  Any VDK API calls
     *   should be performed at the end of "Run()."
     */
}

/* ========================================================================== */
