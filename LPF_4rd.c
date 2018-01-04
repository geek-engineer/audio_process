#include "LPF.h"

#include <stdlib.h> // For malloc/free
#include <string.h> // For memset
#include <stdio.h>		// For test case I/O

int16_t LPF_quarter_coefficients[20] = // the filter coefficients is 1/4 of sample frequency
{//this is float point coefficients multiply by 32767
	//inverse chebyshev filter
	//0.3125fs gain0.95
	11771, 20652, 11771, -7569, -2550,// b0, b1, b2, a1, a2
	16384, 14684, 16384, -959, -17745,// b0, b1, b2, a1, a2

	//0.3125fs gain1.03
	12762, 22391, 12762, -7569, -2550,// b0, b1, b2, a1, a2
	16384, 14684, 16384, -959, -17745// b0, b1, b2, a1, a2
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

 uint32_t LPF_filterBlock( LPFType * pThis, int16_t * pInput, int16_t * pOutput, uint32_t count )
{
	LPF_executionState executionState;          // The executionState structure holds call data, minimizing stack reads and writes
	if( ! count ) return 0;                         // If there are no input samples, return immediately
	executionState.pInput = pInput;                 // Pointers to the input and output buffers that each call to filterBiquad() will use
	executionState.pOutput = pOutput;               // - pInput and pOutput can be equal, allowing reuse of the same memory.
	executionState.count = count;                   // The number of samples to be processed
	executionState.pState = pThis->state;                   // Pointer to the biquad's internal state and coefficients.
	executionState.nb_channel = pThis->nb_channel;          //number of audio channel
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
	 int16_t w0, x0, w1[2], w2[2];
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
	 int16_t *pInput  = pExecState->pInput;
	 int16_t *pOutput = pExecState->pOutput;
	 int32_t count = pExecState->count;
	 int16_t accumulator;
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
		 accumulator  = w2[i] * a2 >> 15;
		 accumulator += w1[i] * a1 >> 15;
		 accumulator += x0 ;

		 w0 = accumulator ;

		 // Run feedforward part of filter
		 accumulator  = w0 * b0 >> 15;
		 accumulator += w1[i] * b1 >> 15;
		 accumulator += w2[i] * b2 >> 15;

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
//
// void LPF_filterBiquad( LPF_executionState * pExecState )
//{
//	// Read state variables
//	int32_t w0, x0;
//	int32_t w1, w2;
//	int32_t w1 = pExecState->pState[0];
//	int32_t w2 = pExecState->pState[1];
//
//	// Read coefficients into work registers
//	int16_t b0 = *(pExecState->pCoefficients++);
//	int16_t b1 = *(pExecState->pCoefficients++);
//	int16_t b2 = *(pExecState->pCoefficients++);
//	int16_t a1 = *(pExecState->pCoefficients++);
//	int16_t a2 = *(pExecState->pCoefficients++);
//
//	// Read source and target pointers
//	int16_t *pInput  = pExecState->pInput;
//	int16_t *pOutput = pExecState->pOutput;
//	int32_t count = pExecState->count;
//	int32_t accumulator;
//
//	// Loop for all samples in the input buffer
//	while( count-- )
//	{
//
//		// Read input sample
//
//		x0 = *(pInput++);
//
//		// Run feedback part of filter
//		accumulator  = w2 * a2 >>15;
//		accumulator += w1 * a1 >>15;
//		accumulator += x0 ;
//
//		w0 = accumulator ;
//
//		// Run feedforward part of filter
//		accumulator  = w0 * b0 >>15;
//		accumulator += w1 * b1 >>15;
//		accumulator += w2 * b2 >>15;
//
//		w2 = w1;		// Shuffle history buffer
//		w1 = w0;
//
//		// Write output
//		*(pOutput++) = accumulator ;
//	}
//
//	// Write state variables
//	*(pExecState->pState++) = w1;
//	*(pExecState->pState++) = w2;
//
//}

