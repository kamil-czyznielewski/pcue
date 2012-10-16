*********************************************************************************************

ADSP-BF537 EZ-KIT Lite C Based AD1854/AD1871 TALKTHRU

Analog Devices, Inc.
DSP Division
Three Technology Way
P.O. Box 9106
Norwood, MA 02062

Date Created:	12/14/04  	GO

____________________________________________________________________________________________

This directory contains an example ADSP-BF537 subroutine that demonstrates the 
initialization of the link between SPORT0 on the ADSP-BF537 and the AD1854/AD1871 
stereo ADC/DAC.  This is done by initializing the link in I2S mode and implementing 
a simple talk-through routine.  

Files contained in this directory:

readme.txt			this file
main.c				C file containing the main program and variable declaration
Initialize.c		C file containing all initialisation routines
ISR.c				C file containing the interrupt service routine for 
					SPORT0_RX
Process_data.c			C file for processing incoming data
Talkthrough.h			C header file containing prototypes and macros
C_Talkthrough_I2S.dpj		VisualDSP++ project file
____________________________________________________________________________________________


CONTENTS

I.	FUNCTIONAL DESCRIPTION
II.	IMPLEMENTATION DESCRIPTION
III.	OPERATION DESCRIPTION


I.    FUNCTIONAL DESCRIPTION

The Talkthru demo demonstrates the initialization of SPORT0 to establish a link 
between the ADSP-BF537 and the AD1854/AD1871 /DAC.
 
The program simply sets up the SPORT0 to receive/transmit audio samples from the 
ADC/DAC.  Audio samples received from the ADC are moved into the processors receive 
buffer, using DMA.  The samples are processed by the ADSP-BF537 and place in the 
transmit buffer.  In turn, the transmit buffer is used to transmit data to the DAC.
This results in a simple talk-through where data is moved in and out of the processor
without performing any computations on the data.


II.   IMPLEMENTATION DESCRIPTION

The Initialization module initializes:

1. PORTF setup 
2. Audio ADC/DAC Reset
3. SPORT0 setup
4. DMA setup
5. Interrupt configuration
6. Enable SPORT and DMA

III.  OPERATION DESCRIPTION

- Open the project "C_Talkthrough_I2S.dpj" in an ADSP-BF537 EZ-KIT Lite session in the 
  VisualDSP Integrated Development Environment (IDDE).
- Under the "Project" tab, select "Build Project" (program is then loaded 
	automatically into DSP).
- Turn ALL pins of SW7 ONand turn pins 1,2 of SW8 ON and 3-6 of SW8 OFF on the 
  ADSP-BF537 EZ-KIT Lite.
- Setup an input source (such as a radio) to the audio in jack and an output 
	source (such as headphones) to the audio out jack of the EZ-KIT Lite.  See the
	ADSP-BF537 EZ-KIT Lite User's Manual for more information on setting up the
	hardware.
- Select "Run" from the "Debug" tab on the menu bar of VisualDSP.
- Listen to the operation of the talk-through.
