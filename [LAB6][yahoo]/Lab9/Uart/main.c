// *****************************************************************************/

#include "ccblkfn.h"
#include "sysreg.h"
#include "Uart0.h"
#include "Uart1.h"

#include <cDEFBF537.h>
#include <sys/exception.h>
#include "ccblkfn.h"
#include "sysreg.h"
#include <stdarg.h>
#include <string.h> 

//--------------------------------------------------------------------------//
// Function:	main														//
//--------------------------------------------------------------------------//
void main(void)
{
	int c;
	UART0_initialize(115200); //ustaw parametry interfejsu UART0
	
	while (1)
	{
		c = UART0_getc(); //odbierz jeden znak
		UART0_putc('#'); //wyœlij #
		UART0_putc(c); //wyœlij odebrany znak
	};
}

