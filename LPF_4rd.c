#include "LPF.h"

#include <stdlib.h> // For malloc/free
#include <string.h> // For memset
#include <stdio.h>		// For test case I/O

float LPF_coefficients[20] =
{
    //0.225fs
    0.44557046962709895, 0.5931514302167308, 0.4455704696270989, 0.2930302634636882, -0.08886351613808602,// b0, b1, b2, a1, a2
    0.25, -0.039197412587311055, 0.25, 0.7330004866846964, -0.5924339949361398,// b0, b1, b2, a1, a2
    12888, 25776, 12888, 20216, -7079,// b0 Q17(0.0983), b1 Q17(0.197), b2 Q17(0.0983), a1 Q14(1.23), a2 Q14(-0.432)
    8192, 16384, 8192, 19943, -12110// b0 Q17(0.0625), b1 Q17(0.125), b2 Q17(0.0625), a1 Q14(1.22), a2 Q14(-0.739)
};

int16_t LPF_quarter_coefficients[20] = // the filter coefficients is 1/4 of sample frequency for 16 bit data
{
    // Scaled for a 16x16:32 Direct form 2 IIR filter with: 
    // Output shift = 15
    // Input shift = 11
    // Central shift = 14
    // Input  has a maximum value of 1, scaled by 2^15
    // Output has a maximum value of 1.5815081782301172, scaled by 2^14
    // 0.07937fs gain0.963

    13240, 26479, 13240, 24020, -9293,// b0 Q18(0.0505), b1 Q18(0.101), b2 Q18(0.0505), a1 Q14(1.47), a2 Q14(-0.567)
    8192, 16384, 8192, 25199, -13149,// b0 Q18(0.0313), b1 Q18(0.0625), b2 Q18(0.0313), a1 Q14(1.54), a2 Q14(-0.803)
    13240, 26479, 13240, 24020, -9293,// b0 Q18(0.0505), b1 Q18(0.101), b2 Q18(0.0505), a1 Q14(1.47), a2 Q14(-0.567)
    8192, 16384, 8192, 25199, -13149// b0 Q18(0.0313), b1 Q18(0.0625), b2 Q18(0.0313), a1 Q14(1.54), a2 Q14(-0.803)

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
 
//     if(volume >= Max_volume) volume = Max_volume;
 
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
    executionState.pCoefficients = LPF_quarter_coefficients;    // Each call to filterBiquad() will advance pState and pCoefficients to the next biquad

	// The 1st call to filter1_filterBiquad() reads from the caller supplied input buffer and writes to the output buffer.
	// The remaining calls to filterBiquad() recycle the same output buffer, so that multiple intermediate buffers are not required.

	LPF_filterBiquad1( &executionState );		// Run biquad #0
	executionState.pInput = executionState.pOutput;         // The remaining biquads will now re-use the same output buffer.
	LPF_filterBiquad1( &executionState );		// Run biquad #1
	executionState.pInput = executionState.pOutput;

//	executionState.pCoefficients = LPF_quarter_coefficients;
	LPF_filterBiquad2( &executionState );		// Run biquad #2
	executionState.pInput = executionState.pOutput;
	LPF_filterBiquad2( &executionState );		// Run biquad #3

	// At this point, the caller-supplied output buffer will contain the filtered samples and the input buffer will contain the unmodified input samples.
	return count;		// Return the number of samples processed, the same as the number of input samples

}


  void LPF_filterBiquad1( LPF_executionState * pExecState )
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
		 accumulator  = (long)w2[i] * a2;// >> pExecState->coefficient_shift;
		 accumulator += (long)w1[i] * a1;// >> pExecState->coefficient_shift;
		 accumulator += (long)x0 << 11;

		 w0 = accumulator >> 14 ;

		 // Run feedforward part of filter
		 accumulator  = (long)w0 * b0;// >> pExecState->coefficient_shift;
		 accumulator += (long)w1[i] * b1;// >> pExecState->coefficient_shift;
		 accumulator += (long)w2[i] * b2 ;//>> pExecState->coefficient_shift;

		 w2[i] = w1[i];		 // Shuffle history buffer
		 w1[i] = w0;

		 // Write output
		 *(pOutput++) = accumulator >> 15 ;
	 }

	 // Write state variables
	 *(pExecState->pState++) = w1[0];
	 *(pExecState->pState++) = w2[0];
	 *(pExecState->pState++) = w1[1];
	 *(pExecState->pState++) = w2[1];

 }
  
  
   void LPF_filterBiquad2( LPF_executionState * pExecState )
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
          accumulator  = (long)w2[i] * a2;// >> pExecState->coefficient_shift;
          accumulator += (long)w1[i] * a1;// >> pExecState->coefficient_shift;
          accumulator += (long)x0 << 11;
  
          w0 = accumulator>>14 ;
  
          // Run feedforward part of filter
          accumulator  = (long)w0 * b0;// >> pExecState->coefficient_shift;
          accumulator += (long)w1[i] * b1;// >> pExecState->coefficient_shift;
          accumulator += (long)w2[i] * b2 ;//>> pExecState->coefficient_shift;
  
          w2[i] = w1[i];      // Shuffle history buffer
          w1[i] = w0;
  
          // Write output
          *(pOutput++) = accumulator >>15 ;
      }
  
      // Write state variables
      *(pExecState->pState++) = w1[0];
      *(pExecState->pState++) = w2[0];
      *(pExecState->pState++) = w1[1];
      *(pExecState->pState++) = w2[1];
  
  }

