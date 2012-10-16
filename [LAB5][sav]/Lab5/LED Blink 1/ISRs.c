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

	*pPORTFIO_TOGGLE = ucActive_LED;
	
	// shift old LED pattern by one
	if(sLight_Move_Direction)
	{
		if((ucActive_LED = ucActive_LED >> 1) == 0x0020)
		{
			ucActive_LED = 0x0040;
			sLight_Move_Direction = ~sLight_Move_Direction;
		}
	}
	else
	{
		if((ucActive_LED = ucActive_LED << 1) == 0x1000)
		{
			ucActive_LED = 0x0800;
			sLight_Move_Direction = ~sLight_Move_Direction;
		}
	}

	// write new LED pattern to PORTF LEDs
	*pPORTFIO_TOGGLE = ucActive_LED;
	
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
	sLight_Move_Direction = ~sLight_Move_Direction;
	ucActive_LED = 0x0020;
}

