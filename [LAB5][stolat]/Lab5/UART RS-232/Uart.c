// ****************************************************************************************************

#include "Uart.h"

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


int getSCLK(unsigned int _CLKIN)
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

int getCCLK(unsigned int _CLKIN)
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

void UART_initialize(int rate)
{	
	
	//UART_init_tx_dma();
	//UART_init_rx_dma();		
	
	int divisor,_SCLK;
	
	BIT_SET(*pPORTF_FER,0); // UART RX
	BIT_SET(*pPORTF_FER,1); // UART TX		
	BIT_CLEAR(*pPORT_MUX, 3); // UART RX/TX
		
	_SCLK = getSCLK(CLOCK_IN); 
	divisor = (_SCLK + (int) rate*8  ) / ( (int) rate*16); // round to nearest
		
	*pUART0_GCTL = UCEN; // enable UART clock. 
	*pUART0_LCR = DLAB;
	*pUART0_DLL = divisor;
	*pUART0_DLH = divisor>>8;	
	*pUART0_LCR = WLS(8); // Clear DLAB again and set UART frame to 8 bits, no parity, 1 stop bit. 		
		
}

// ****************************************************************************************************

void UART_disable(void)
{
	UART_waitForTransferCompletion();
	*pUART0_GCTL = 0;	
}

// ****************************************************************************************************

void UART_waitForTransferCompletion(void)
{	
	while (!(*pUART0_LSR & TEMT)) { }; // wait
	ssync();
}

// ****************************************************************************************************

void UART_putc(char c)
{
	while (!(*pUART0_LSR & THRE)) { }; //wait
	*pUART0_THR = c;
}


// ****************************************************************************************************

void UART_puts(char *c)
{
	while (*c) {	
		UART_putc(*c);
		c++;
	}
}

// ****************************************************************************************************

void UART_write(char *c, int size)
{	
	for ( ; size>0 ; --size,++c) UART_putc(*c);			
}

// ****************************************************************************************************

char UART_getc(void)
{
	char c;
	while (!(*pUART0_LSR & DR)) { }; //wait
	c = *pUART0_RBR;
	*pPORTFIO_CLEAR = 0x0FC4;
	return c;
}

// ****************************************************************************************************

int UART_gets(char *str, int max)
{
	int i;
	for(i = 0; i < max; i++) {
		str[i] = UART_getc();
		if (str[i] == 13) {
			return i+1;
		}
	}
	return max;
}

// ****************************************************************************************************
