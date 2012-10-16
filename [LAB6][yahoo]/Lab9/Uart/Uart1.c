// ****************************************************************************************************

#include "Uart1.h"

// ****************************************************************************************************


#include <cDEFBF537.h>
#include <sys/exception.h>
#include "ccblkfn.h"
#include "sysreg.h"
#include <stdarg.h>
#include <string.h> 

// ****************************************************************************************************

#define CLOCK_IN 25000000 // CLKIN = 25MHz

// ****************************************************************************************************


int getSCLK1(unsigned int _CLKIN)
{			
	unsigned short tempPLLCTL, tempPLLDIV;	
	unsigned int _MSEL, _SSEL, _DF, _VCO, _SCLK;
	
	tempPLLCTL = *pPLL_CTL;
	tempPLLDIV = *pPLL_DIV;

	//get MSEL, SSEL, DF
	_MSEL = ((tempPLLCTL & 0x7E00) >> 9);
	_SSEL = tempPLLDIV & 0x000f;
	_DF   = tempPLLCTL & 0x0001;
	
	if(_DF == 1) _CLKIN = _CLKIN/2;
	
	_VCO  = _MSEL * _CLKIN;
	return _VCO/_SSEL;  	
}

// ****************************************************************************************************

int getCCLK1(unsigned int _CLKIN)
{			
	unsigned short tempPLLCTL, tempPLLDIV;	
	unsigned int _MSEL, _CSEL, _DF, _VCO, _SCLK;
	
	tempPLLCTL = *pPLL_CTL;
	tempPLLDIV = *pPLL_DIV;

	//get MSEL, SSEL, DF
	_MSEL = ((tempPLLCTL & 0x7E00) >> 9);
	_CSEL = (tempPLLDIV>>4) & 0x03;
	_DF   = tempPLLCTL & 0x0001;
	
	if(_DF == 1) _CLKIN = _CLKIN/2;
	
	_VCO  = _MSEL * _CLKIN;
	return _VCO >> _CSEL;  	
}

// ****************************************************************************************************

#define BIT_SET(x,y) (x) |= (1<<(y))
#define BIT_CLEAR(x,y) (x) &= ~(1<<(y))


// ****************************************************************************************************

void UART1_initialize(int rate)
{	
	
	//UART1_init_tx_dma();
	//UART1_init_rx_dma();		
	
	int divisor,_SCLK;
	
	BIT_SET(*pPORTF_FER,2); // UART RX
	BIT_SET(*pPORTF_FER,3); // UART TX		
	BIT_CLEAR(*pPORT_MUX, 4); // UART RX/TX
		
	_SCLK = getSCLK1(CLOCK_IN); 
	divisor = (_SCLK + (int) rate*8  ) / ( (int) rate*16); // round to nearest
		
	*pUART1_GCTL = UCEN; // enable UART clock. 
	*pUART1_LCR = DLAB;
	*pUART1_DLL = divisor;
	*pUART1_DLH = divisor>>8;	
	*pUART1_LCR = WLS(8); // Clear DLAB again and set UART frame to 8 bits, no parity, 1 stop bit. 		
		
}

// ****************************************************************************************************

void UART1_disable(void)
{
	UART1_waitForTransferCompletion();
	*pUART1_GCTL = 0;	
}

// ****************************************************************************************************

void UART1_waitForTransferCompletion(void)
{	
	while (!(*pUART1_LSR & TEMT)) { }; // wait
	ssync();
}

// ****************************************************************************************************

void UART1_putc(char c)
{
	while (!(*pUART1_LSR & THRE)) { }; //wait
	*pUART1_THR = c;
}


// ****************************************************************************************************

void UART1_puts(char *c)
{
	while (*c) {	
		UART1_putc(*c);
		c++;
	}
}

// ****************************************************************************************************

void UART1_write(char *c, int size)
{	
	for ( ; size>0 ; --size,++c) UART1_putc(*c);			
}

// ****************************************************************************************************

char UART1_checkc(void)
{
	return (*pUART1_LSR & DR) ? 1 : 0;
}

// ****************************************************************************************************


char UART1_getc(void)
{
	char c;
	while (!(*pUART1_LSR & DR)) { }; //wait
	c = *pUART1_RBR;
	return c;
}

// ****************************************************************************************************

int UART1_gets(char *str, int max)
{
	int i;
	for(i = 0; i < max; i++) {
		str[i] = UART1_getc();
		if (str[i] == 13) {
			return i+1;
		}
	}
	return max;
}

// ****************************************************************************************************
