/* =============================================================================
 *
 *  Description: This is a C implementation for Thread Discard_Server_ThreadType
 *
 * -----------------------------------------------------------------------------
 *  Comments:
 *
 * ===========================================================================*/

/* Get access to any of the VDK features & datatypes used */
#include "Discard_Server_ThreadType.h"
#include "charcb.h"
#include "IPv4_util.h"

#include <stddef.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>

#define MAX_WORKERS 3
charcb_t dbs[MAX_WORKERS];

void
Discard_Server_ThreadType_RunFunction(void **inPtr)
{
    /* Put the thread's "main" Initialization HERE */
  int listenfd;
  struct sockaddr_in saddr;
  int c;
  char *discard_area_start=NULL;
  
  discard_area_start =(char*)malloc(MAX_WORKERS * sizeof(char) * DISCARD_BUFFER_SIZE);
  if(discard_area_start == NULL)
  {
  	printf("failed to allocate memory to discard server\n");
  	return;
  }
  
  printf("Discard Server running (port 9)\n");

  // First acquire our socket for listening for connections
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    fprintf(stderr, "Discard Server: socket create failed\n");
    exit(200);
  }
  
  // set up stream info
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = htonl(INADDR_ANY);
  saddr.sin_port = htons(9);     // Discard service well-known port number

  if (bind(listenfd, (struct sockaddr*)&saddr, sizeof(saddr)) == -1) {
    fprintf(stderr, "Discard Server: socket bind failed");
    exit(201);
  }

  // Put socket into listening mode
  if (listen(listenfd, MAX_WORKERS) == -1) {
    fprintf(stderr, "Discard Server: listen failed");
    exit(202);
  }
  
  // mark all dbs entries free and set up buffer addresses and lengths
  for (c = 0; c < MAX_WORKERS; c += 1) {
    dbs[c].socket = -1;
    dbs[c].buff = (char*)(discard_area_start + (c * DISCARD_BUFFER_SIZE));
    dbs[c].buff_len = DISCARD_BUFFER_SIZE;
  }
  
  // loop forever awaiting connections on port 9
  while (1)
  {
    charcb_t* p;
    charcb_t  db;
    
    memset(&db, 0, sizeof(charcb_t));
    db.clilen = sizeof(struct sockaddr);
    db.socket = accept(listenfd, (struct sockaddr*)&db.cliaddr, &db.clilen);
    
    if (db.socket == -1) {
      fprintf(stderr, "Discard Server: accept() error\n");
      exit(203);
    } else {
      struct VDK_ThreadCreationBlock TCB = {
        kDiscard_Worker_ThreadType,
        (VDK_ThreadID)0,
        0,
        (VDK_Priority)0
      };
        
      printf("Discard connection from IP %s Port %u - ", 
              IPv4_straddr(ntohl(db.cliaddr.sin_addr.s_addr)),
              ntohs(db.cliaddr.sin_port));
    
      // find a free db
      for (c = 0; c < MAX_WORKERS; c += 1) {
        if (dbs[c].socket == -1) {
          p = &dbs[c];
          memcpy(p, &db, offsetof(charcb_t, nextchar));
          break;
        }
      }
      
      if (c == MAX_WORKERS) {
        // too many live connections - just close this one
        close(db.socket);
        printf("dropped\n");
      } else {
        printf("accepted\n");
        TCB.user_data_ptr = (void*)p;
        if (VDK_CreateThreadEx(&TCB) == UINT_MAX) {
          fprintf(stderr, "Discard Server: failed to create worker thread\n");
          exit(205);
        }
      }
    }
  }

    /* Put the thread's exit from "main" HERE */
    /* A thread is automatically Destroyed when it exits its run function */
}

int
Discard_Server_ThreadType_ErrorFunction(void **inPtr)
{

    /* TODO - Put this thread's error handling code HERE */

      /* The default ErrorHandler kills the thread */

	VDK_DestroyThread(VDK_GetThreadID(), false);
	return 0;
}

void
Discard_Server_ThreadType_InitFunction(void **inPtr, VDK_ThreadCreationBlock *pTCB)
{
    /* Put code to be executed when this thread has just been created HERE */

    /* This routine does NOT run in new thread's context.  Any non-static thread
     *   initialization should be performed at the beginning of "Run()."
     */
}

void
Discard_Server_ThreadType_DestroyFunction(void **inPtr)
{
    /* Put code to be executed when this thread is destroyed HERE */

    /* This routine does NOT run in the thread's context.  Any VDK API calls
     *   should be performed at the end of "Run()."
     */
}

/* ========================================================================== */
