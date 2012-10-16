#include "BF537 Flags.h"

//--------------------------------------------------------------------------//
// Function:	Timer0_ISR													//
//																			//
// Parameters:	None														//
//																			//
// Return:		None														//
//																			//
// Description:	This ISR is executed every time Timer0 expires.				//
//				The old LED pattern is shifted by one; the direction		//
//				depends on the state of sLight_Move_Direction, which is		//
//				changed in PORTF_IntA_ISR.									//
//--------------------------------------------------------------------------//
EX_INTERRUPT_HANDLER(Timer0_ISR)
{
	// confirm interrupt handling
	*pTIMER_STATUS = 0x0001;

	// shift old LED pattern by one
	if(sChanged_Mode)
	{
		timerCounter++;
		if (timerCounter == 10)
		{				
			*pPORTFIO_TOGGLE = ucActive_LED;
			if (ucActive_LED == 0x0A80)
			{
				ucActive_LED = 0x0540;
			}
			else
			{
				ucActive_LED = 0x0A80;
			}
			*pPORTFIO_TOGGLE = ucActive_LED;
			timerCounter = 0;
		}
	}
	else
	{
		if((ucActive_LED = ucActive_LED << 1) == 0x1000) ucActive_LED = 0x0020;
		*pPORTFIO_TOGGLE = ucActive_LED;
	}	
	
}

//--------------------------------------------------------------------------//
// Function:	PORTF_IntA_ISR			
//
//																			//
// Parameters:	None														//
//																			//
// Return:		None														//
//																			//
// Description:	This ISR is called every time the button connected to PORTF2//
//				is pressed.													//
//				The state of flag sLight_Move_Direction is changed, so the	//
//				shift-direction for the LED pattern in Timer0_ISR changes.	//
//--------------------------------------------------------------------------//
EX_INTERRUPT_HANDLER(PORTF_IntA_ISR)
{
	// confirm interrupt handling and clear flags
	*pPORTFIO_CLEAR = 0x0FC4;

	// toggle direction of moving light
	sChanged_Mode = ~sChanged_Mode;
	
	// w zmienionym trybie migaja wszystkie diody
	if (sChanged_Mode)
	{
		timerCounter = 0;
		ucActive_LED = 0x0A80;
		*pPORTFIO_TOGGLE = ucActive_LED;
	}
	else
	{
		ucActive_LED = 0x0020;
	}
}

