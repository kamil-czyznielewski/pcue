#include <lwip/sockets.h>
typedef struct _charcb 
{
    int    socket;
    struct sockaddr_in cliaddr;
    int    clilen;
    int    nextchar;
    char*  buff;
    int    buff_len;
} charcb_t;

#define CHARGEN_BUFFER_SIZE ( 8 * 1024)
#define ECHO_BUFFER_SIZE ( 8 * 1024)
#define DISCARD_BUFFER_SIZE ( 8 * 1024)
