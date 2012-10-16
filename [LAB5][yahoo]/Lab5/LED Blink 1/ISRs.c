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

  //w zale¿noœci od kierunku ruchu...
	if(sLight_Move_Direction)
	{
    //pi³ka o 1 w lewo
		ucActive_LED = ucActive_LED<<1;
		
    //je¿eli dobiliœmy do lwewj granicy, to zawracamy
    if (ucActive_LED==0x0800)
			sLight_Move_Direction = ~sLight_Move_Direction;
	} else {
		//pi³ka o 1 w prawo
    ucActive_LED = ucActive_LED>>1;
		
    //je¿eli dobilismy do prawej granicy to zawracamy
    if (ucActive_LED==0x0040)
			sLight_Move_Direction = ~sLight_Move_Direction;
	}

	//przepisujemy obliczony stan do rejestru steruj¹cego diodami
	*pPORTFIO = ucActive_LED;
	
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
	
  //przycisk poza zmian¹ kierunku resetuje tak¿e stan pi³ki na maksymalnie prawy (?)
  ucActive_LED = 0x0040;
}

