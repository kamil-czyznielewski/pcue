/* =============================================================================
 *
 *  Description: This is a C implementation for Thread CharGen_Server_ThreadType
 *
 * -----------------------------------------------------------------------------
 *  Comments:
 *
 * ===========================================================================*/

/* Get access to any of the VDK features & datatypes used */
#include "CharGen_Server_ThreadType.h"
#include "charcb.h"
#include "IPv4_util.h"

#include <stddef.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>

#define MAX_WORKERS 3
charcb_t cbs[MAX_WORKERS];

void
CharGen_Server_ThreadType_RunFunction(void **inPtr)
{
  /* Put the thread's "main" Initialization HERE */
  int listenfd;
  struct sockaddr_in saddr;
  int c;
  char *chargen_area_start=NULL;
  
  chargen_area_start =(char*)malloc(MAX_WORKERS * sizeof(char) * CHARGEN_BUFFER_SIZE);

  if(chargen_area_start == NULL)
  {
  	printf("failed to allocate memory to chargen server\n");
  	return;
  }
  printf("CharGen Server running (port 19)\n");

  // First acquire our socket for listening for connections
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    fprintf(stderr, "CharGen Server: socket create failed\n");
    exit(100);
  }
  
  // set up stream info
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = htonl(INADDR_ANY);
  saddr.sin_port = htons(19);     // Chargen service well-known port number

  if (bind(listenfd, (struct sockaddr*)&saddr, sizeof(saddr)) == -1) {
    fprintf(stderr, "CharGen Server: socket bind failed");
    exit(101);
  }

  // Put socket into listening mode
  if (listen(listenfd, MAX_WORKERS) == -1) {
    fprintf(stderr, "CharGen Server: listen failed");
    exit(102);
  }
  
  // mark all cbs entries free and set up buffer addresses and lengths
  for (c = 0; c < MAX_WORKERS; c += 1) {
    cbs[c].socket = -1;
    cbs[c].buff = (char*)(chargen_area_start + (c * CHARGEN_BUFFER_SIZE));
    cbs[c].buff_len = CHARGEN_BUFFER_SIZE;
  }
  
  // loop forever awaiting connections on port 19 (eg % telnet <IP addr> 19)
  while (1)
  {
    charcb_t* p;
    charcb_t  cb;
    
    memset(&cb, 0, sizeof(charcb_t));
    cb.clilen = sizeof(struct sockaddr);
    cb.socket = accept(listenfd, (struct sockaddr*)&cb.cliaddr, &cb.clilen);
    
    if (cb.socket == -1) 
    {
      fprintf(stderr, "CharGen Server: accept() error\n");
      exit(103);
    } 
    else 
    {
      struct VDK_ThreadCreationBlock TCB = {
        kCharGen_Worker_ThreadType,
        (VDK_ThreadID)0,
        0,
        (VDK_Priority)0
      };
        
      printf("CharGen connection from IP %s Port %u - ", 
              IPv4_straddr(ntohl(cb.cliaddr.sin_addr.s_addr)),
              ntohs(cb.cliaddr.sin_port));
    
      // find a free cb
      for (c = 0; c < MAX_WORKERS; c += 1) {
        if (cbs[c].socket == -1) {
          p = &cbs[c];
          memcpy(p, &cb, offsetof(charcb_t, nextchar));
          break;
        }
      }
      
      if (c == MAX_WORKERS) {
        // too many live connections - just close this one
        close(cb.socket);
        printf("dropped\n");
      } else {
        printf("accepted\n");
        TCB.user_data_ptr = (void*)p;
        if (VDK_CreateThreadEx(&TCB) == UINT_MAX) {
          fprintf(stderr, "CharGen Server: failed to create worker thread\n");
          exit(105);
        }
      }
    }
  }


    /* Put the thread's exit from "main" HERE */
    /* A thread is automatically Destroyed when it exits its run function */
}

int
CharGen_Server_ThreadType_ErrorFunction(void **inPtr)
{

    /* TODO - Put this thread's error handling code HERE */

      /* The default ErrorHandler kills the thread */

	VDK_DestroyThread(VDK_GetThreadID(), false);
	return 0;
}

void
CharGen_Server_ThreadType_InitFunction(void **inPtr, VDK_ThreadCreationBlock *pTCB)
{
    /* Put code to be executed when this thread has just been created HERE */

    /* This routine does NOT run in new thread's context.  Any non-static thread
     *   initialization should be performed at the beginning of "Run()."
     */
}

void
CharGen_Server_ThreadType_DestroyFunction(void **inPtr)
{
    /* Put code to be executed when this thread is destroyed HERE */

    /* This routine does NOT run in the thread's context.  Any VDK API calls
     *   should be performed at the end of "Run()."
     */
}

/* ========================================================================== */
