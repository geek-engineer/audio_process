#include "LPF.h"

#include <stdlib.h> // For malloc/free
#include <string.h> // For memset
#include <stdio.h>		// For test case I/O

float LPF_coefficients[20] =
{
	//inverse chebyshev filter
	//0.25fs gain0.95
    0.17409548597834018, 0.2592354342967524, 0.17409548597834018, 0.5922284983795371, -0.13071247034859523,// b0, b1, b2, a1, a2
    0.25, 0.039504286779635814, 0.24999999999999997, 0.9310339819652323, -0.5716417215410786,// b0, b1, b2, a1, a2

	//inverse chebyshev filter
	//0.25fs gain0.95
    0.17409548597834018, 0.2592354342967524, 0.17409548597834018, 0.5922284983795371, -0.13071247034859523,// b0, b1, b2, a1, a2
    0.25, 0.039504286779635814, 0.24999999999999997, 0.9310339819652323, -0.5716417215410786// b0, b1, b2, a1, a2
};

int16_t LPF_quarter_coefficients[20] = // the filter coefficients is 1/4 of sample frequency for 16 bit data
{//this is float point coefficients multiply by 32767
    //inverse chebyshev filter
    //0.3125fs gain0.95
    11771, 20652, 11771, -7569, -2550,// b0, b1, b2, a1, a2
    16384, 14684, 16384, -959, -17745,// b0, b1, b2, a1, a2

    //0.3125fs gain1.03
    12762, 22391, 12762, -7569, -2550,// b0, b1, b2, a1, a2
    16384, 14684, 16384, -959, -17745// b0, b1, b2, a1, a2
};

int16_t LPF_quarter_coefficients16[20] = // the filter coefficients is 1/4 of sample frequency for 16 bit data
{//this is float point coefficients multiply by 32767
	//inverse chebyshev filter
	//0.3125fs gain0.95
	11771, 20652, 11771, -7569, -2550,// b0, b1, b2, a1, a2
	16384, 14684, 16384, -959, -17745,// b0, b1, b2, a1, a2

	//0.3125fs gain1.03
	12762, 22391, 12762, -7569, -2550,// b0, b1, b2, a1, a2
	16384, 14684, 16384, -959, -17745// b0, b1, b2, a1, a2
};

int16_t LPF_quarter_coefficients24[20] = // the filter coefficients is 1/4 of sample frequency for 24 bit data
{//this is float point coefficients multiply by 128
	//inverse chebyshev filter
	//0.3125fs gain0.95
	46, 80, 46, -29, -9,// b0, b1, b2, a1, a2
	64, 57, 64, -3, -69,// b0, b1, b2, a1, a2

	//0.3125fs gain1.03
	64, 87, 49, -29, -9,// b0, b1, b2, a1, a2
	64, 57, 64, -3, -69// b0, b1, b2, a1, a2
};

LPFType *LPF_create( void )
{
	LPFType *result = (LPFType *)malloc( sizeof( LPFType ) );	// Allocate memory for the object
	LPF_init( result );											// Initialize it
	return result;												// Return the result
}

void LPF_destroy( LPFType *pObject )
{
	free( pObject );
}

 void LPF_init( LPFType * pThis )
{
	LPF_reset( pThis );

}

 void LPF_reset( LPFType * pThis )
{
	memset( &pThis->state, 0, sizeof( pThis->state ) ); // Reset state to 0
	pThis->output = 0;									// Reset output
	pThis->nb_channel = 0;

}

 void LPF_volume(uint16_t volume, uint16_t bit_depth)
 {
     //init the LPF coefficients
     int i, j;
 
     if(volume >= Max_volume) volume = Max_volume;
 
     j =((bit_depth == 24) ? 128 : 32767) * volume / Max_volume;
     i = 0;
     while(i < sizeof(LPF_coefficients)/sizeof(LPF_coefficients[0])) {
         LPF_quarter_coefficients[i] = LPF_coefficients[i] * j;
         //printf("%d\n", LPF_quarter_coefficients24[i]);
         i++;
     }
 }

 uint32_t LPF_filterBlock( LPFType * pThis, int32_t * pInput, int32_t * pOutput, uint32_t count )
{
	LPF_executionState executionState;          // The executionState structure holds call data, minimizing stack reads and writes
	if( ! count ) return 0;                         // If there are no input samples, return immediately
	executionState.pInput = pInput;                 // Pointers to the input and output buffers that each call to filterBiquad() will use
	executionState.pOutput = pOutput;               // - pInput and pOutput can be equal, allowing reuse of the same memory.
	executionState.count = count;                   // The number of samples to be processed
	executionState.pState = pThis->state;                   // Pointer to the biquad's internal state and coefficients.
	executionState.nb_channel = pThis->nb_channel;          //number of audio channel
	executionState.coefficient_shift = 31 - pThis->bit_depth;
//	if (pThis->bit_depth == 16) {
//		executionState.pCoefficients = LPF_quarter_coefficients16;    // Each call to filterBiquad() will advance pState and pCoefficients to the next biquad
//	} else if(pThis->bit_depth == 24) {
//		executionState.pCoefficients = LPF_quarter_coefficients24;    // Each call to filterBiquad() will advance pState and pCoefficients to the next biquad
//	}
    executionState.pCoefficients = LPF_quarter_coefficients;    // Each call to filterBiquad() will advance pState and pCoefficients to the next biquad

	// The 1st call to filter1_filterBiquad() reads from the caller supplied input buffer and writes to the output buffer.
	// The remaining calls to filterBiquad() recycle the same output buffer, so that multiple intermediate buffers are not required.

	LPF_filterBiquad( &executionState );		// Run biquad #0
	executionState.pInput = executionState.pOutput;         // The remaining biquads will now re-use the same output buffer.
	LPF_filterBiquad( &executionState );		// Run biquad #1
//	executionState.pInput = executionState.pOutput;
//
//	executionState.pCoefficients = LPF_quarter_coefficients;
//	LPF_filterBiquad( &executionState );		// Run biquad #2
//	executionState.pInput = executionState.pOutput;
//	LPF_filterBiquad( &executionState );		// Run biquad #3

	// At this point, the caller-supplied output buffer will contain the filtered samples and the input buffer will contain the unmodified input samples.
	return count;		// Return the number of samples processed, the same as the number of input samples

}


  void LPF_filterBiquad( LPF_executionState * pExecState )
 {
	 // Read state variables
	 int32_t w0, x0, w1[2], w2[2];
	 w1[0] = pExecState->pState[0];
	 w2[0] = pExecState->pState[1];
	 w1[1] = pExecState->pState[2];
	 w2[1] = pExecState->pState[3];
	 // Read coefficients into work registers
	 int16_t b0 = *(pExecState->pCoefficients++);
	 int16_t b1 = *(pExecState->pCoefficients++);
	 int16_t b2 = *(pExecState->pCoefficients++);
	 int16_t a1 = *(pExecState->pCoefficients++);
	 int16_t a2 = *(pExecState->pCoefficients++);

	 // Read source and target pointers
	 int32_t *pInput  = pExecState->pInput;
	 int32_t *pOutput = pExecState->pOutput;
	 int32_t count = pExecState->count;
	 int32_t accumulator;
	 int16_t i = 0;
	 int16_t ch_sel;
	 ch_sel = (pExecState->nb_channel == 2)? 1: 0;
	 // Loop for all samples in the input buffer
	 while( count-- )
	 {
		 //stereo or mono
		 i = count & ch_sel;

		 // Read input sample
		 x0 = *(pInput++);

		 // Run feedback part of filter
		 accumulator  = w2[i] * a2 >> pExecState->coefficient_shift;
		 accumulator += w1[i] * a1 >> pExecState->coefficient_shift;
		 accumulator += x0 ;

		 w0 = accumulator ;

		 // Run feedforward part of filter
		 accumulator  = w0 * b0 >> pExecState->coefficient_shift;
		 accumulator += w1[i] * b1 >> pExecState->coefficient_shift;
		 accumulator += w2[i] * b2 >> pExecState->coefficient_shift;

		 w2[i] = w1[i];		 // Shuffle history buffer
		 w1[i] = w0;

		 // Write output
		 *(pOutput++) = accumulator ;
	 }

	 // Write state variables
	 *(pExecState->pState++) = w1[0];
	 *(pExecState->pState++) = w2[0];
	 *(pExecState->pState++) = w1[1];
	 *(pExecState->pState++) = w2[1];

 }

