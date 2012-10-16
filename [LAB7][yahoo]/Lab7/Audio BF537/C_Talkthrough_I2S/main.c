/*****************************************************************************
**																			**
**	 Name: 	BF537 C Talkthrough I2S										**	
**																			**
******************************************************************************
Analog Devices, Inc.  All rights reserved.

File Name:	Main.c

Date Modified:	12/14/04		Rev 1.0
		01/11/05		Rev 1.1 Updated to work on Rev 1.1 of the ADSP-BF537 EZ-Kit

Hardware:	ADSP-BF537 EZ-KIT Board Rev 1.1
			Users of ADSP-BF537 EZ-KIT Board Rev 1.0 must change the SPORT to 
			generate internal clock and frame sync

Purpose:	This program sets up the ADSP-BF537 to reset the ADC and DAC.  
			The data to/from the DAC/ADC are transfered over SPORT0 in I2S mode.		                                          
				                                                                          
Program Parameters:                                                                                       
                                                                                                          

******************************************************************************/   
#include "Talkthrough.h"
#include <sysreg.h>
#include <ccblkfn.h>


/*****************************************************************************

Variables																
																			
Description:	The variables ChannelxLeftIn and ChannelxRightIn contain 	
		the data coming from the codec ADC (AD1871).  The (processed)		
		playback data are written into the variables 			
		ChannelxLeftOut and ChannelxRightOut respectively, which 	
		are then sent back to the DAC (AD1854) in the SPORT0 ISR.  		
				
******************************************************************************/
// left input data from AD1871
int iChannel0LeftIn, iChannel1LeftIn;
// right input data from AD1871
int iChannel0RightIn, iChannel1RightIn;
// left ouput data for AD1854	
int iChannel0LeftOut, iChannel1LeftOut;
// right ouput data for AD1854
int iChannel0RightOut, iChannel1RightOut;
// SPORT0 DMA transmit buffer
int iTxBuffer1[2];
// SPORT0 DMA receive buffer
int iRxBuffer1[2];


//--------------------------------------------------------------------------//
// Function:	main														//
//																			//
// Description:	After calling a few initalization routines, main() just 	//
//				waits in a loop forever.  The code to process the incoming  //
//				data can be placed in the function Process_Data() in the 	//
//				file "Process_Data.c".										//
//--------------------------------------------------------------------------//
void main(void)
{
	Init_Flags();
	Audio_Reset();
	Init_Sport0();
	Init_DMA();
	Init_Interrupts();
	Enable_DMA_Sport0();

	while(1);
}
