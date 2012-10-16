#include "Talkthrough.h"

//--------------------------------------------------------------------------//
// Function:	Process_Data()                                                //
//                                                                          //
// Description: This function is called from inside the SPORT0 ISR every    //
//				time a complete audio frame has been received. The new            //
//				input samples can be found in the variables iChannel0LeftIn,      //
//				iChannel0RightIn, iChannel1LeftIn and iChannel1RightIn            //
//				respectively. The processed	data should be stored in              //
//				iChannel0LeftOut, iChannel0RightOut, iChannel1LeftOut,            //
//				iChannel1RightOut, iChannel2LeftOut and	iChannel2RightOut         //
//				respectively.                                                     //
//--------------------------------------------------------------------------//


void effectNone();
void enhancer();
void bandstop();
void lowpass1();
void lowpass2();
void denoiser();
void reverb();
void echo();
void compression();



#define EFFECT_NONE                0
#define EFFECT_ENHANCER            1
#define EFFECT_FIR_BANDSTOP        2
#define EFFECT_FIR_LOWPASS1        3
#define EFFECT_FIR_LOWPASS2        4
#define EFFECT_DENOISER            5
#define EFFECT_REVERB              6
#define EFFECT_ECHO                7
#define EFFECT_DYNAMIC_COMPRESSION 8


int bufferL[100];
int bufferR[100];


int effect = EFFECT_NONE;


void Process_Data(void)
{        
    //select effect
    switch (effect)
    {
		case EFFECT_NONE:                //no effect
			effectNone();
			break;

		case EFFECT_ENHANCER:            //exciter/enhancer
			enhancer();
			break;

		case EFFECT_FIR_BANDSTOP:        //bandstop
			bandstop();
			break;

		case EFFECT_FIR_LOWPASS1:        //lowpass1
			lowpass1();
			break;

		case EFFECT_FIR_LOWPASS2:        //lowpass2
			lowpass2();
			break;

		case EFFECT_DENOISER:            //noise canceller
			denoiser();
			break;

		case EFFECT_REVERB:              //reverb
			reverb();
			break;
		
		case EFFECT_ECHO:                //echo
			echo();
			break;

		case EFFECT_DYNAMIC_COMPRESSION: //dynamic compression
			compression();
			break;
    }

}

//-----------------------------------------------------------------------------

void effectNone()
{
    //simple forwarding of a sample
	iChannel0LeftOut = iChannel0LeftIn;
	iChannel0RightOut = iChannel0RightIn;
}

//-----------------------------------------------------------------------------

void enhancer()
{
    int attenuationDivisor = 10;
    
    //adding phase inverted and attenuated right source signal to left channel
    iChannel0LeftOut = iChannel0LeftIn - (iChannel0RightIn/attenuationDivisor);
    
    //vice versa
    iChannel0RightOut = iChannel0RightIn - (iChannel0LeftIn/attenuationDivisor);
}

//-----------------------------------------------------------------------------

//bandstop 300Hz-8kHz
int const FIR_bandstop[100] = 
{
    42,    69,    93,   109,   112,   100,    72,    34,    -6,   -34,
   -37,    -2,    72,   177,   294,   394,   447,   427,   329,   167,
   -23,  -185,  -262,  -209,    -7,   321,   713,  1075,  1305,  1316,
  1067,   582,   -47,  -668, -1103, -1189,  -825,   -13,  1129,  2370,
  3399,  3887,  3553,  2234,   -66, -3159, -6690,-10189,-13154,-15139,
 86999,-15139,-13154,-10189, -6690, -3159,   -66,  2234,  3553,  3887,
  3399,  2370,  1129,   -13,  -825, -1189, -1103,  -668,   -47,   582,
  1067,  1316,  1305,  1075,   713,   321,    -7,  -209,  -262,  -185,
   -23,   167,   329,   427,   447,   394,   294,   177,    72,    -2,
   -37,   -34,    -6,    34,    72,   100,   112,   109,    93,    69
};


void bandstop()
{
    int i, accL, accR;
    
	//shift buffer
	for (i=99; i>0; --i)
	{
		bufferL[i] = bufferL[i-1];
		bufferR[i] = bufferR[i-1];
	}

	//insert current sample into buffer (reduced to 8 bits)
	bufferL[0] = iChannel0LeftIn>>16;
	bufferR[0] = iChannel0RightIn>>16;

	//compute weighted sum for each channel
	accL=0;
	accR=0;
	for (i=0; i<100; ++i)
	{
		accL += bufferL[i]*FIR_bandstop[i];
		accR += bufferR[i]*FIR_bandstop[i];
	}

	//output data
	iChannel0LeftOut   = accL;		
	iChannel0RightOut  = accR;	
}

//-----------------------------------------------------------------------------

//lowpass 4.8KHz
int const FIR_lowpass1[100] = 
{
     0,     5,    11,    16,    20,    24,    25,    24,    20,    12,
     0,   -15,   -33,   -51,   -68,   -80,   -85,   -81,   -66,   -38,
     0,    47,    99,   150,   195,   225,   235,   219,   174,   100,
     0,  -120,  -250,  -377,  -486,  -562,  -589,  -554,  -446,  -261,
     0,   332,   721,  1150,  1596,  2034,  2438,  2783,  3047,  3212,
  3269,  3212,  3047,  2783,  2438,  2034,  1596,  1150,   721,   332,
     0,  -261,  -446,  -554,  -589,  -562,  -486,  -377,  -250,  -120,
     0,   100,   174,   219,   235,   225,   195,   150,    99,    47,
     0,   -38,   -66,   -81,   -85,   -80,   -68,   -51,   -33,   -15,
     0,    12,    20,    24,    25,    24,    20,    16,    11,     5
};

void lowpass1()
{
    int i, accL, accR;
    
	//shift buffer
	for (i=99; i>0; --i)
	{
		bufferL[i] = bufferL[i-1];
		bufferR[i] = bufferR[i-1];
	}

	//insert current sample into buffer (reduced to 8 bits)
	bufferL[0] = iChannel0LeftIn>>16;
	bufferR[0] = iChannel0RightIn>>16;

	//compute weighted sum for each channel
	accL=0;
	accR=0;
	for (i=0; i<100; ++i)
	{
		accL += bufferL[i]*FIR_lowpass1[i];
		accR += bufferR[i]*FIR_lowpass1[i];
	}

	//output data
	iChannel0LeftOut   = accL;		
	iChannel0RightOut  = accR;	
}

//-----------------------------------------------------------------------------

//lowpass 24kHz
int const FIR_lowpass2[100] = 
{
     0,    17,     0,   -20,     0,    24,     0,   -30,     0,    39,
     0,   -50,     0,    63,     0,   -80,     0,   101,     0,  -125,
     0,   153,     0,  -186,     0,   225,     0,  -271,     0,   325,
     0,  -389,     0,   467,     0,  -563,     0,   686,     0,  -848,
     0,  1075,     0, -1424,     0,  2038,     0, -3447,     0, 10415,
 16375, 10415,     0, -3447,     0,  2038,     0, -1424,     0,  1075,
     0,  -848,     0,   686,     0,  -563,     0,   467,     0,  -389,
     0,   325,     0,  -271,     0,   225,     0,  -186,     0,   153,
     0,  -125,     0,   101,     0,   -80,     0,    63,     0,   -50,
     0,    39,     0,   -30,     0,    24,     0,   -20,     0,    17
};

void lowpass2()
{
    int i, accL, accR;

    //shift buffer
	for (i=99; i>0; --i)
	{
		bufferL[i] = bufferL[i-1];
		bufferR[i] = bufferR[i-1];
	}
	
	//insert current sample into buffer (reduced to 8 bits)
	bufferL[0] = iChannel0LeftIn>>16;
	bufferR[0] = iChannel0RightIn>>16;

	//compute weighted sum for each channel
	accL=0;	
	accR=0;	
	for (i=0; i<100; ++i)
	{
		accL += bufferL[i]*FIR_lowpass2[i];
		accR += bufferR[i]*FIR_lowpass2[i];
	}

	//output data
	iChannel0LeftOut   = accL;		
	iChannel0RightOut  = accR;	
}

//-----------------------------------------------------------------------------

void denoiser()
{
    //noise canceller cutoff level
	int cutoff = 500; 
    
	if (iChannel0LeftIn < cutoff && iChannel0LeftIn > -1*cutoff)
	{
		iChannel0LeftOut = 0;
	} else {
		iChannel0LeftOut = iChannel0LeftIn;
	}

	if (iChannel0RightIn < cutoff && iChannel0RightIn > -1*cutoff)
	{
		iChannel0RightOut = 0;
	} else {
		iChannel0RightOut = iChannel0RightIn;
	}
}

//-----------------------------------------------------------------------------

#define SAMPLES_PER_PHASE 5000 //about 2/5ms for f=48kHz

int buffer[SAMPLES_PER_PHASE];
int bufferOffset=0;

void reverb()
{
    /* multiple fading echoes */
    
    //joining channels
    int jointStereo = (iChannel0RightIn + iChannel0LeftIn)/2;
    
    //saving result to the buffer
    bufferOffset = (bufferOffset + 1) % SAMPLES_PER_PHASE;
    buffer[bufferOffset] = jointStereo;
    
    //mixing current input with samples from the past
    int output = jointStereo/2
               + buffer[(bufferOffset + SAMPLES_PER_PHASE*3/4) % SAMPLES_PER_PHASE]/4
               + buffer[(bufferOffset + SAMPLES_PER_PHASE*2/4) % SAMPLES_PER_PHASE]/8
               + buffer[(bufferOffset + SAMPLES_PER_PHASE*1/4) % SAMPLES_PER_PHASE]/16;
    
    //output
    iChannel0RightOut = output;
    iChannel0LeftOut = output;
    
}

//-----------------------------------------------------------------------------

void echo()
{
    //joining channels
    int jointStereo = (iChannel0RightIn + iChannel0LeftIn)/2;
    
    //saving result to the buffer
    bufferOffset = (bufferOffset + 1) % SAMPLES_PER_PHASE;
    buffer[bufferOffset] = jointStereo/2;
    
    //mixing current input with sample one phase backward
    int output = jointStereo/2 + buffer[(bufferOffset + 1) % SAMPLES_PER_PHASE /* here is the oldest entry in buffer */]/2;
    
    //output
    iChannel0RightOut = output;
    iChannel0LeftOut = output;
}

//-----------------------------------------------------------------------------

void compression()
{
	//compression activation level (threshold)
	int threshold = 5000;
	
	//compression ratio
	int ratio = 2; ///for 2:1 compression
    
	if (iChannel0LeftIn < threshold && iChannel0LeftIn > -1*threshold)
	{
		iChannel0LeftOut = iChannel0LeftIn;
	} else {
	    if (iChannel0LeftIn < 0)
	    {
	        iChannel0LeftOut = threshold - (abs(iChannel0LeftIn-threshold)/ratio);
	    } else {
	        iChannel0LeftOut = threshold + (abs(iChannel0LeftIn-threshold)/ratio);
	    }
	}

	if (iChannel0RightIn < threshold && iChannel0RightIn > -1*threshold)
	{
		iChannel0RightOut = iChannel0RightIn;
	} else {
	    if (iChannel0RightIn < 0)
	    {
	        iChannel0RightOut = threshold - (abs(iChannel0RightIn-threshold)/ratio);
	    } else {
	        iChannel0RightOut = threshold + (abs(iChannel0RightIn-threshold)/ratio);
	    }
	}
}
