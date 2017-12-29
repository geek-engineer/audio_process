#include "LPF.h"

#include <stdlib.h> // For malloc/free
#include <string.h> // For memset
#include <stdio.h>		// For test case I/O

int16_t LPF_quarter_coefficients[10] = // the filter coefficients is 1/4 of sample frequency
{//this is float point coefficients multiply by 32767
	10067, 16012, 10067, -239, -2300, // b0, b1, b2, a1, a2
	16384, 6598, 16384, 10825, -18361 // b0, b1, b2, a1, a2
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

}

 uint32_t LPF_filterBlock( LPFType * pThis, int16_t * pInput, int16_t * pOutput, uint32_t count )
{
	LPF_executionState executionState;          // The executionState structure holds call data, minimizing stack reads and writes
	if( ! count ) return 0;                         // If there are no input samples, return immediately
	executionState.pInput = pInput;                 // Pointers to the input and output buffers that each call to filterBiquad() will use
	executionState.pOutput = pOutput;               // - pInput and pOutput can be equal, allowing reuse of the same memory.
	executionState.count = count;                   // The number of samples to be processed
	executionState.pState = pThis->state;                   // Pointer to the biquad's internal state and coefficients.
	executionState.pCoefficients = LPF_quarter_coefficients;    // Each call to filterBiquad() will advance pState and pCoefficients to the next biquad

	// The 1st call to filter1_filterBiquad() reads from the caller supplied input buffer and writes to the output buffer.
	// The remaining calls to filterBiquad() recycle the same output buffer, so that multiple intermediate buffers are not required.

	LPF_filterBiquad( &executionState );		// Run biquad #0
	executionState.pInput = executionState.pOutput;         // The remaining biquads will now re-use the same output buffer.

	LPF_filterBiquad( &executionState );		// Run biquad #1
	executionState.pInput = executionState.pOutput;
	executionState.pCoefficients = LPF_quarter_coefficients;
	LPF_filterBiquad( &executionState );		// Run biquad #2
	executionState.pInput = executionState.pOutput;
	LPF_filterBiquad( &executionState );		// Run biquad #3

	// At this point, the caller-supplied output buffer will contain the filtered samples and the input buffer will contain the unmodified input samples.
	return count;		// Return the number of samples processed, the same as the number of input samples

}

 void LPF_filterBiquad( LPF_executionState * pExecState )
{
	// Read state variables
	int32_t w0, x0;
	int32_t w1 = pExecState->pState[0];
	int32_t w2 = pExecState->pState[1];

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
	int32_t accumulator;

	// Loop for all samples in the input buffer
	while( count-- )
	{
		// Read input sample
		x0 = *(pInput++);

		// Run feedback part of filter
		accumulator  = w2 * a2 >>15;
		accumulator += w1 * a1 >>15;
		accumulator += x0 ;

		w0 = accumulator ;

		// Run feedforward part of filter
		accumulator  = w0 * b0 >>15;
		accumulator += w1 * b1 >>15;
		accumulator += w2 * b2 >>15;

		w2 = w1;		// Shuffle history buffer
		w1 = w0;

		// Write output
		*(pOutput++) = accumulator ;
	}

	// Write state variables
	*(pExecState->pState++) = w1;
	*(pExecState->pState++) = w2;

}

 int LPF_filterInChunks( LPFType * pThis, int16_t * pInput, int16_t * pOutput, uint32_t length )
{
 	int processedLength = 0;
	uint32_t chunkLength, outLength;
//	static long random = 0x6428734; // Use pseudo-random number generator to split input into small random length chunks.
	while( length > 0)
	{
		chunkLength = length;//random & 0xf;											// Choose random chunkLength from 0 - 15
		if( chunkLength > length ) chunkLength = length;					// Limit chunk length to the number of remaining samples
		outLength = LPF_filterBlock( pThis,  pInput, pOutput, chunkLength );		// Filter the block and determine the number of returned samples
		pOutput += outLength;												// Update the output pointer
		processedLength += outLength;										// Update the total number of samples output
		pInput += chunkLength;												// Update the input pointer
		length -= chunkLength;												// Update the number of samples remaining
//		random = random + 0x834f4527;										// Cycle the simple random number generator
	}
	return processedLength;													// Return the number of samples processed

}


