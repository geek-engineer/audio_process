#include "LPF.h"

#include <stdlib.h> // For malloc/free
#include <string.h> // For memset
#include <stdio.h>    // For test case I/O

const float LPF_coefficients[36] =
{
    //The coefficients calculted by "Iowa Hills IIR Filters", http://iowahills.com/
    //Sampling freq:48k, OmegaC:0.2136, Ripple:0.21db, Stop band: 110db, 11 poles, Gain:L-0.45
    //Sect 0
    -1.000000000000000000,1.442057469797411120,-0.528411212954901566,0.109270553351304464,-0.134347101945947123,0.109270553351304464,   //a0,a1,a2,b0,b1,b2,
    //Sect 1
    -1.000000000000000000,1.459955369517176080,-0.596878591469297115,0.151313045432695498,-0.169127368299195169,0.151313045432695498,   //a0,a1,a2,b0,b1,b2,
    //Sect 2
    -1.000000000000000000,1.492894076262163860,-0.705664020039408535,0.174259501532445765,-0.141070512664366987,0.174259501532445765,   //a0,a1,a2,b0,b1,b2,
    //Sect 3
    -1.000000000000000000,1.540130303157246190,-0.824690073529609102,0.141048570987286487,-0.004654314550280140,0.141048570987286487,   //a0,a1,a2,b0,b1,b2,
    //Sect 4
    -1.000000000000000000,1.607626211174523870,-0.942085635349023676,0.091919494914015659,0.142255482979030040,0.091919494914015659     //a0,a1,a2,b0,b1,b2,
};

int16_t LPF_shift_coefficients[36] =
{
     -82,  185,  -68,   14,  -17,   14,
    -150,  187,  -76,   19,  -22,   19,
    -150,  191,  -90,   22,  -18,   22,
    -150,  197, -106,   18,   -1,   18,
    -150,  206, -121,   12,   18,   12,
       0,    0,    0,    0,    0,    0,
};

LPFType *LPF_create( void )
{
  LPFType *result = (LPFType *)malloc( sizeof( LPFType ) ); // Allocate memory for the object
  LPF_init( result );                     // Initialize it
  return result;                        // Return the result
}

void LPF_destroy( LPFType *pObject )
{
//  free( pObject );
}

 void LPF_init( LPFType * pThis )
{
  LPF_reset( pThis );

}

 void LPF_reset( LPFType * pThis )
{
  memset( &pThis->state, 0, sizeof( pThis->state ) ); // Reset state to 0
    int i = 0;
    while ( i < 24) {
        pThis->state[i] = 0;
        i++;
    }
  pThis->output = 0;                  // Reset output
  pThis->nb_channel = 0;

}

void LPF_volume(uint16_t volume, uint16_t bit_depth)
{
    //init the LPF coefficients
    int i, j;

    if(volume >= Max_volume) volume = Max_volume;

     j = 256 * volume / Max_volume;
     i = 3;
     while(i < 6) {
         LPF_shift_coefficients[i] = LPF_coefficients[i] * j;
         i++;
     }
     j = 512 * volume / Max_volume;
     i = 9;
     while(i < 12) {
         LPF_shift_coefficients[i] = LPF_coefficients[i] * j;
         i++;
     }
     j = 512 * volume / Max_volume;
     i = 15;
     while(i < 18) {
         LPF_shift_coefficients[i] = LPF_coefficients[i] * j;
         i++;
     }
     j = 256* volume / Max_volume;
     i = 21;
     while(i < 24) {
         LPF_shift_coefficients[i] = LPF_coefficients[i] * j;
         i++;
     }
     j = 256 * volume / Max_volume;
     i = 27;
     while(i < 30) {
         LPF_shift_coefficients[i] = LPF_coefficients[i] * j;
         i++;
     }
     j = 256 * volume / Max_volume;
     i = 33;
     while(i < 36) {
         LPF_shift_coefficients[i] = LPF_coefficients[i] * j;
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
  executionState.step = &pThis->step;

    executionState.pCoefficients = LPF_shift_coefficients;    // Each call to filterBiquad() will advance pState and pCoefficients to the next biquad

  // The 1st call to filter1_filterBiquad() reads from the caller supplied input buffer and writes to the output buffer.
  // The remaining calls to filterBiquad() recycle the same output buffer, so that multiple intermediate buffers are not required.

  LPF_filterBiquad0( &executionState );   // Run biquad #0
  executionState.pInput = executionState.pOutput;         // The remaining biquads will now re-use the same output buffer.
    executionState.step = &pThis->step;
  LPF_filterBiquad1( &executionState );   // Run biquad #1
  executionState.pInput = executionState.pOutput;
  executionState.step = &pThis->step;
  LPF_filterBiquad2( &executionState );   // Run biquad #2
  executionState.pInput = executionState.pOutput;
    executionState.step = &pThis->step;
  LPF_filterBiquad3( &executionState );   // Run biquad #3
    executionState.pInput = executionState.pOutput;
  executionState.step = &pThis->step;
  LPF_filterBiquad4( &executionState );   // Run biquad #4

//  IF MORE ORDER NEEDED
//    executionState.pInput = executionState.pOutput;
//  executionState.step = &pThis->step;
//  LPF_filterBiquad5( &executionState );   // Run biquad #5

    // At this point, the caller-supplied output buffer will contain the filtered samples and the input buffer will contain the unmodified input samples.

  return count;   // Return the number of samples processed, the same as the number of input samples
}


void LPF_filterBiquad0( LPF_executionState * pExecState )
{
    // Read state variables
    int32_t w0, x0, w1[2], w2[2];
    int16_t b0, b1, b2, a0, a1, a2;
    int32_t *pInput, *pOutput, count, accumulator;
    int16_t i, ch_sel;

    w1[0] = pExecState->pState[*pExecState->step * 4 + 0];
    w2[0] = pExecState->pState[*pExecState->step * 4 + 1];
    w1[1] = pExecState->pState[*pExecState->step * 4 + 2];
    w2[1] = pExecState->pState[*pExecState->step * 4 + 3];

    // Read coefficients into work registers
    a0 = *(pExecState->pCoefficients++);
    a1 = *(pExecState->pCoefficients++);
    a2 = *(pExecState->pCoefficients++);
    b0 = *(pExecState->pCoefficients++);
    b1 = *(pExecState->pCoefficients++);
    b2 = *(pExecState->pCoefficients++);

    // Read source and target pointers
    pInput  = pExecState->pInput;
    pOutput = pExecState->pOutput;
    count = pExecState->count;
    i = 0;
    ch_sel = (pExecState->nb_channel == 2)? 1: 0;
    // Loop for all samples in the input buffer
    while( count-- )
    {
     //stereo or mono
     i = count & ch_sel;

     // Read input sample
     x0 = *(pInput++);

     // Run feedback part of filter
     accumulator  = (long)w2[i] * a2;
     accumulator += (long)w1[i] * a1 ;
     accumulator += (long)(x0 * a0) >> 2 ;

     w0 = accumulator >> 7 ;

     // Run feedforward part of filter
     accumulator  = (long)w0 * b0;
     accumulator += (long)w1[i] * b1;
     accumulator += (long)w2[i] * b2;

     w2[i] = w1[i]; // Shuffle history buffer
     w1[i] = w0;

     // Write output
     *(pOutput++) = accumulator >> 7;
    }

    // Write state variables

    pExecState->pState[*pExecState->step * 4 + 0] = w1[0];
    pExecState->pState[*pExecState->step * 4 + 1] = w2[0];
    pExecState->pState[*pExecState->step * 4 + 2] = w1[1];
    pExecState->pState[*pExecState->step * 4 + 3] = w2[1];
    *pExecState->step = *pExecState->step + 1;
}

void LPF_filterBiquad1( LPF_executionState * pExecState )
{
    // Read state variables
    int32_t w0, x0, w1[2], w2[2];
    int16_t b0, b1, b2, a0, a1, a2;
    int32_t *pInput, *pOutput, count, accumulator;
    int16_t i, ch_sel;

    w1[0] = pExecState->pState[*pExecState->step * 4 + 0];
    w2[0] = pExecState->pState[*pExecState->step * 4 + 1];
    w1[1] = pExecState->pState[*pExecState->step * 4 + 2];
    w2[1] = pExecState->pState[*pExecState->step * 4 + 3];

    // Read coefficients into work registers
    a0 = *(pExecState->pCoefficients++);
    a1 = *(pExecState->pCoefficients++);
    a2 = *(pExecState->pCoefficients++);
    b0 = *(pExecState->pCoefficients++);
    b1 = *(pExecState->pCoefficients++);
    b2 = *(pExecState->pCoefficients++);

    // Read source and target pointers
    pInput  = pExecState->pInput;
    pOutput = pExecState->pOutput;
    count = pExecState->count;
    i = 0;
    ch_sel = (pExecState->nb_channel == 2)? 1: 0;
    // Loop for all samples in the input buffer
    while( count-- )
    {
     //stereo or mono
     i = count & ch_sel;

     // Read input sample
     x0 = *(pInput++);

     // Run feedback part of filter
     accumulator  = (long)w2[i] * a2;
     accumulator += (long)w1[i] * a1 ;
     accumulator -= (long)(x0 * a0) >> 2;

     w0 = accumulator >> 7 ;

     // Run feedforward part of filter
     accumulator  = (long)w0 * b0;
     accumulator += (long)w1[i] * b1;
     accumulator += (long)w2[i] * b2;

     w2[i] = w1[i]; // Shuffle history buffer
     w1[i] = w0;

     // Write output
     *(pOutput++) = accumulator >> 7;
    }

    // Write state variables

    pExecState->pState[*pExecState->step * 4 + 0] = w1[0];
    pExecState->pState[*pExecState->step * 4 + 1] = w2[0];
    pExecState->pState[*pExecState->step * 4 + 2] = w1[1];
    pExecState->pState[*pExecState->step * 4 + 3] = w2[1];
    *pExecState->step = *pExecState->step + 1;
}

void LPF_filterBiquad2( LPF_executionState * pExecState )
{
    // Read state variables
    int32_t w0, x0, w1[2], w2[2];
    int16_t b0, b1, b2, a0, a1, a2;
    int32_t *pInput, *pOutput, count, accumulator;
    int16_t i, ch_sel;

    w1[0] = pExecState->pState[*pExecState->step * 4 + 0];
    w2[0] = pExecState->pState[*pExecState->step * 4 + 1];
    w1[1] = pExecState->pState[*pExecState->step * 4 + 2];
    w2[1] = pExecState->pState[*pExecState->step * 4 + 3];

    // Read coefficients into work registers
    a0 = *(pExecState->pCoefficients++);
    a1 = *(pExecState->pCoefficients++);
    a2 = *(pExecState->pCoefficients++);
    b0 = *(pExecState->pCoefficients++);
    b1 = *(pExecState->pCoefficients++);
    b2 = *(pExecState->pCoefficients++);

    // Read source and target pointers
    pInput  = pExecState->pInput;
    pOutput = pExecState->pOutput;
    count = pExecState->count;
    i = 0;
    ch_sel = (pExecState->nb_channel == 2)? 1: 0;
    // Loop for all samples in the input buffer
    while( count-- )
    {
     //stereo or mono
     i = count & ch_sel;

     // Read input sample
     x0 = *(pInput++);

     // Run feedback part of filter
     accumulator  = (long)w2[i] * a2;
     accumulator += (long)w1[i] * a1 ;
     accumulator -= (long)(x0 * a0) >> 2;

     w0 = accumulator >> 7 ;

     // Run feedforward part of filter
     accumulator  = (long)w0 * b0;
     accumulator += (long)w1[i] * b1;
     accumulator += (long)w2[i] * b2;

     w2[i] = w1[i]; // Shuffle history buffer
     w1[i] = w0;

     // Write output
     *(pOutput++) = accumulator >> 7;
    }

    // Write state variables

    pExecState->pState[*pExecState->step * 4 + 0] = w1[0];
    pExecState->pState[*pExecState->step * 4 + 1] = w2[0];
    pExecState->pState[*pExecState->step * 4 + 2] = w1[1];
    pExecState->pState[*pExecState->step * 4 + 3] = w2[1];
    *pExecState->step = *pExecState->step + 1;
}

 void LPF_filterBiquad3( LPF_executionState * pExecState )
{
    // Read state variables
    int32_t w0, x0, w1[2], w2[2];
    int16_t b0, b1, b2, a0, a1, a2;
    int32_t *pInput, *pOutput, count, accumulator;
    int16_t i, ch_sel;

    w1[0] = pExecState->pState[*pExecState->step * 4 + 0];
    w2[0] = pExecState->pState[*pExecState->step * 4 + 1];
    w1[1] = pExecState->pState[*pExecState->step * 4 + 2];
    w2[1] = pExecState->pState[*pExecState->step * 4 + 3];

    // Read coefficients into work registers
    a0 = *(pExecState->pCoefficients++);
    a1 = *(pExecState->pCoefficients++);
    a2 = *(pExecState->pCoefficients++);
    b0 = *(pExecState->pCoefficients++);
    b1 = *(pExecState->pCoefficients++);
    b2 = *(pExecState->pCoefficients++);

    // Read source and target pointers
    pInput  = pExecState->pInput;
    pOutput = pExecState->pOutput;
    count = pExecState->count;
    i = 0;
    ch_sel = (pExecState->nb_channel == 2)? 1: 0;
    // Loop for all samples in the input buffer
    while( count-- )
    {
     //stereo or mono
     i = count & ch_sel;

     // Read input sample
     x0 = *(pInput++);

     // Run feedback part of filter
     accumulator  = (long)w2[i] * a2;
     accumulator += (long)w1[i] * a1 ;
     accumulator -= (long)(x0 * a0) >> 1;

     w0 = accumulator >> 7 ;

     // Run feedforward part of filter
     accumulator  = (long)w0 * b0;
     accumulator += (long)w1[i] * b1;
     accumulator += (long)w2[i] * b2;

     w2[i] = w1[i]; // Shuffle history buffer
     w1[i] = w0;

     // Write output
     *(pOutput++) = accumulator >> 6;
    }

    // Write state variables

    pExecState->pState[*pExecState->step * 4 + 0] = w1[0];
    pExecState->pState[*pExecState->step * 4 + 1] = w2[0];
    pExecState->pState[*pExecState->step * 4 + 2] = w1[1];
    pExecState->pState[*pExecState->step * 4 + 3] = w2[1];
    *pExecState->step = *pExecState->step + 1;
}

void LPF_filterBiquad4( LPF_executionState * pExecState )
{
     // Read state variables
     int32_t w0, x0, w1[2], w2[2];
     int16_t b0, b1, b2, a0, a1, a2;
     int32_t *pInput, *pOutput, count, accumulator;
     int16_t i, ch_sel;

     w1[0] = pExecState->pState[*pExecState->step * 4 + 0];
     w2[0] = pExecState->pState[*pExecState->step * 4 + 1];
     w1[1] = pExecState->pState[*pExecState->step * 4 + 2];
     w2[1] = pExecState->pState[*pExecState->step * 4 + 3];

     // Read coefficients into work registers
     a0 = *(pExecState->pCoefficients++);
     a1 = *(pExecState->pCoefficients++);
     a2 = *(pExecState->pCoefficients++);
     b0 = *(pExecState->pCoefficients++);
     b1 = *(pExecState->pCoefficients++);
     b2 = *(pExecState->pCoefficients++);

     // Read source and target pointers
     pInput  = pExecState->pInput;
     pOutput = pExecState->pOutput;
     count = pExecState->count;
     i = 0;
     ch_sel = (pExecState->nb_channel == 2)? 1: 0;
     // Loop for all samples in the input buffer
     while( count-- )
     {
      //stereo or mono
      i = count & ch_sel;

      // Read input sample
      x0 = *(pInput++);

      // Run feedback part of filter
      accumulator  = (long)w2[i] * a2;
      accumulator += (long)w1[i] * a1 ;
      accumulator -= (long)(x0 * a0) >> 1;

      w0 = accumulator >> 7 ;

      // Run feedforward part of filter
      accumulator  = (long)w0 * b0;
      accumulator += (long)w1[i] * b1;
      accumulator += (long)w2[i] * b2;

      w2[i] = w1[i]; // Shuffle history buffer
      w1[i] = w0;

      // Write output
      *(pOutput++) = accumulator >> 7;
     }

     // Write state variables

     pExecState->pState[*pExecState->step * 4 + 0] = w1[0];
     pExecState->pState[*pExecState->step * 4 + 1] = w2[0];
     pExecState->pState[*pExecState->step * 4 + 2] = w1[1];
     pExecState->pState[*pExecState->step * 4 + 3] = w2[1];
     *pExecState->step = *pExecState->step + 1;
}
void LPF_filterBiquad5( LPF_executionState * pExecState )
{
     // Read state variables
     int32_t w0, x0, w1[2], w2[2];
     int16_t b0, b1, b2, a0, a1, a2;
     int32_t *pInput, *pOutput, count, accumulator;
     int16_t i, ch_sel;

     w1[0] = pExecState->pState[*pExecState->step * 4 + 0];
     w2[0] = pExecState->pState[*pExecState->step * 4 + 1];
     w1[1] = pExecState->pState[*pExecState->step * 4 + 2];
     w2[1] = pExecState->pState[*pExecState->step * 4 + 3];

     // Read coefficients into work registers
     a0 = *(pExecState->pCoefficients++);
     a1 = *(pExecState->pCoefficients++);
     a2 = *(pExecState->pCoefficients++);
     b0 = *(pExecState->pCoefficients++);
     b1 = *(pExecState->pCoefficients++);
     b2 = *(pExecState->pCoefficients++);

     // Read source and target pointers
     pInput  = pExecState->pInput;
     pOutput = pExecState->pOutput;
     count = pExecState->count;
     i = 0;
     ch_sel = (pExecState->nb_channel == 2)? 1: 0;
     // Loop for all samples in the input buffer
     while( count-- )
     {
      //stereo or mono
      i = count & ch_sel;

      // Read input sample
      x0 = *(pInput++);

      // Run feedback part of filter
      accumulator  = (long)w2[i] * a2;
      accumulator += (long)w1[i] * a1 ;
      accumulator -= (long)(x0 * a0) >> 2;

      w0 = accumulator >> 7 ;

      // Run feedforward part of filter
      accumulator  = (long)w0 * b0;
      accumulator += (long)w1[i] * b1;
      accumulator += (long)w2[i] * b2;

      w2[i] = w1[i]; // Shuffle history buffer
      w1[i] = w0;

      // Write output
      *(pOutput++) = accumulator >> 6;
     }

     // Write state variables

     pExecState->pState[*pExecState->step * 4 + 0] = w1[0];
     pExecState->pState[*pExecState->step * 4 + 1] = w2[0];
     pExecState->pState[*pExecState->step * 4 + 2] = w1[1];
     pExecState->pState[*pExecState->step * 4 + 3] = w2[1];
     *pExecState->step = *pExecState->step + 1;
}


