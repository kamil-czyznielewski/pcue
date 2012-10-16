#include "Talkthrough.h"

//--------------------------------------------------------------------------//
// Function:	Process_Data()												//
//																			//
// Description: This function is called from inside the SPORT0 ISR every 	//
//				time a complete audio frame has been received. The new 		//
//				input samples can be found in the variables iChannel0LeftIn,//
//				iChannel0RightIn, iChannel1LeftIn and iChannel1RightIn 		//
//				respectively. The processed	data should be stored in 		//
//				iChannel0LeftOut, iChannel0RightOut, iChannel1LeftOut,		//
//				iChannel1RightOut, iChannel2LeftOut and	iChannel2RightOut	//
//				respectively.												//
//--------------------------------------------------------------------------//


int bufferL[100];
int bufferR[100];


// bandstop 300Hz - 8kHz
int const FIR[100] = 
{
     42,   69,   93,  109,  112,  100,   72,   34,   -6,  -34,  -37,   -2,   72
,  177,  294,  394,  447,  427,  329,  167,  -23, -185, -262, -209,   -7,  321
,  713, 1075, 1305, 1316, 1067,  582,  -47, -668,-1103,-1189, -825,  -13, 1129, 2370
, 3399, 3887, 3553, 2234,  -66,-3159,-6690 ,-10189 ,-13154 ,-15139,86999, -15139
,-13154,-10189,-6690,-3159,  -66, 2234, 3553, 3887, 3399, 2370, 1129,  -13, -825
,-1189,-1103, -668,  -47,  582, 1067, 1316, 1305, 1075,  713,  321,   -7, -209
, -262, -185,  -23,  167,  329,  427,  447,  394,  294,  177,   72,   -2,  -37
,  -34,   -6,   34,   72,  100,  112,  109,   93,   69
};

/*
// lowpass 4.8KHz
int const FIR[100] = 
{
0,   5,  11,  16,  20,  24,  25,  24,  20,  12,   0, -15, -33, -51, -68, -80, -85, -81
, -66, -38,   0,  47,  99, 150, 195, 225, 235, 219, 174, 100,   0,-120,-250,-377,-486
,-562,-589,-554,-446,-261,   0, 332, 721,1150,1596,2034,2438,2783,3047,3212,3269,3212
,3047,2783,2438,2034,1596,1150, 721, 332,   0,-261,-446,-554,-589,-562,-486,-377,-250
,-120,   0, 100, 174, 219, 235, 225, 195, 150,  99,  47,   0, -38, -66, -81, -85, -80
, -68, -51, -33, -15,   0,  12,  20,  24,  25,  24,  20,  16,  11,   5
};
*/

/*
// lowpass 24kHz
int const FIR[100] = 
{
0,17,0,-20,0,24,0,-30,0,39,0,-50,
0,63,0,-80,0,101,0,-125,0,153,0,-186,
0,225,0,-271,0,325,0,-389,0,467,0,-563,
0,686,0,-848,0,1075,0,-1424,0,2038,0,-3447,
0,10415,16375,10415,0,-3447,0,2038,0,-1424,0,1075,
0,-848,0,686,0,-563,0,467,0,-389,0,325,
0,-271,0,225,0,-186,0,153,0,-125,0,101,
0,-80,0,63,0,-50,0,39,0,-30,0,24,
0,-20,0,17,0
};*/


void PoszerzaczCharakterystkiStereo(void) {
    
	int s = (int)((iChannel0LeftIn - iChannel0RightIn)/2);
    s*=0.1;
    iChannel0LeftOut = iChannel0LeftIn + s;
    iChannel0RightOut = iChannel0RightIn - s;
} 



void KompresorDynamiki(int prog, int wspolczynnik)
{
	int tmpL = iChannel0LeftIn;
    int tmpR = iChannel0RightIn;
		
    
    if (iChannel0LeftIn > prog)
    	tmpL = prog + (iChannel0LeftIn - prog) / wspolczynnik;
    else if (iChannel0LeftIn < -prog)
    	tmpL = -prog + (iChannel0LeftIn + prog) / wspolczynnik;

    if (iChannel0RightIn > prog)
    	tmpR = prog + (iChannel0RightIn - prog) / wspolczynnik;
	else if (iChannel0RightIn < - prog)
    	tmpR = -prog + (iChannel0RightIn + prog) / wspolczynnik;
    
    iChannel0LeftOut = tmpL;
    iChannel0RightOut = tmpR;
} 


void BramkaSzumowa(int prog)
{
	int tmpL = iChannel0LeftIn;
    int tmpR = iChannel0RightIn;
	
    if (iChannel0LeftIn < prog && iChannel0LeftIn > -prog)
        tmpL = 0;
    if (iChannel0RightIn < prog && iChannel0RightIn > -prog)
        tmpR = 0;
    
    iChannel0LeftOut = tmpL;
    iChannel0RightOut = tmpR;
}



#define BUFFOR_SIZE 5000

int Buffor[BUFFOR_SIZE] = { 0 };
int Buffor_Pozycja = 0;

int GetFromBuffer(int index)
{
    return Buffor[(Buffor_Pozycja + index) % BUFFOR_SIZE];
}

void Echo(void)
{
	int value = (iChannel0RightIn + iChannel0LeftIn) / 2;
    Buffor[Buffor_Pozycja] = value;

    iChannel0LeftOut  = value / 2 + GetFromBuffer(1) / 4;
    iChannel0RightOut = iChannel0LeftOut;
   
    Buffor_Pozycja = (Buffor_Pozycja + 1) % BUFFOR_SIZE;
}



void Poglos(void)
{
	int value = (iChannel0RightIn + iChannel0LeftIn) / 2;
    Buffor[Buffor_Pozycja] = value;

    iChannel0LeftOut  = value / 2 + GetFromBuffer(6000) / 4 + GetFromBuffer(5000) / 8 + GetFromBuffer(4000) / 8;
    iChannel0RightOut = iChannel0LeftOut;
   
    Buffor_Pozycja = (Buffor_Pozycja + 1) % BUFFOR_SIZE;
}


void Process_Data(void)
{	
	//BramkaSzumowa(5000);
		
	//KompresorDynamiki(4000000, 8);
	
	//PoszerzaczCharakterystkiStereo();

	//Echo();
	
	Poglos();
	
		
	//iChannel0LeftOut = iChannel0LeftIn;
	//iChannel0RightOut = iChannel0RightIn;
	
	
	/*
	int i, accL, accR;
	
	for (i=99; i>0; --i)
	{
		bufferL[i] = bufferL[i-1];
		bufferR[i] = bufferR[i-1];
	}
	
	bufferL[0] = iChannel0LeftIn>>16;
	bufferR[0] = iChannel0RightIn>>16;
	
	accL=0;	
	accR=0;	
	for (i=0; i<100; ++i)
	{
	  accL += bufferL[i]*FIR[i];
	  accR += bufferR[i]*FIR[i];
	}
	  

	iChannel0LeftOut   = accL;		
	iChannel0RightOut  = accR;
	*/
}
