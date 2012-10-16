#include <stdio.h>
char* IPv4_straddr(unsigned long a)
{
  static char str[20];
  
  sprintf(str, "%d.%d.%d.%d", 
          a >> 24, (a >> 16) & 255, (a >> 8) & 255, a & 255);
  return str;
}
  

