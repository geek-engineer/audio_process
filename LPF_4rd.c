#include "LPF.h"

#include <stdlib.h> // For malloc/free
#include <string.h> // For memset
#include <stdio.h>    // For test case I/O
 float LPF_coefficients[30] =
 {
    //     //10 order elliptic type filter
    //     //a0,a1,a2,b0,b1,b2,
    //Sect 0
    -1.000000000000000000,1.649436517750981100,-0.695913509734196634,0.076723277156391434,-0.107554445726074088,0.076723277156391434,
    //Sect 1
    -1.000000000000000000,1.614269310935006140,-0.769515376233997683,0.232019091276370687,-0.310745790288338153,0.232019091276370687,
    //Sect 2
    -1.000000000000000000,1.573650790546480270,-0.860485349668433819,0.333379927041684565,-0.383534925533793369,0.333379927041684565,
    //Sect 3
    -1.000000000000000000,1.549981301439699700,-0.930917734471075287,0.263966572302143321,-0.151790554403467504,0.263966572302143321,
    //Sect 4
    -1.000000000000000000,1.550877775063175920,-0.979531652700193933,0.131208934725317389,0.160841671651680534,0.13120893472531738,

 };

int16_t LPF_quarter_coefficients[30] = // the filter coefficients is 1/4 of sample frequency for 16 bit data
{
    -128, 211, -89,  118, -165,  118,//a:q(7), b:q(10)*1.5
    -128, 207, -98,  178, -239,  178,//a:q(7), b:q(9) *1.5
    -128, 201,-110,  273, -314,  273,//a:q(7), b:q(9) *1.6
    -128, 198,-119,  405, -233,  405,//a:q(7), b:q(10)*1.5
    -128, 199,-125,  202,  247,  202,//a:q(7), b:q(10)*1.5
};

LPFType *LPF_create( void )
{
  LPFType *result = (LPFType *)malloc( sizeof( LPFType ) ); // Allocate memory for the object
  LPF_init( result );                     // Initialize it
  return result;                        // Return the result
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
  pThis->output = 0;                  // Reset output
  pThis->nb_channel = 0;

}

 void LPF_volume(uint16_t volume, uint16_t bit_depth)
 {
     //init the LPF coefficients
     int i, j;

 //    if(volume >= Max_volume) volume = Max_volume;

     j = 1536 * volume / Max_volume;
     i = 3;
     while(i < 6) {
         LPF_quarter_coefficients[i] = LPF_coefficients[i] * j;
         i++;
     }

     j = 768 * volume / Max_volume;
     i = 9;
     while(i < 12) {
         LPF_quarter_coefficients[i] = LPF_coefficients[i] * j;
         i++;
     }
     j = 819 * volume / Max_volume;
     i = 15;
     while(i < 18) {
         LPF_quarter_coefficients[i] = LPF_coefficients[i] * j;
         i++;
     }
     j = 1536* volume / Max_volume;
     i = 21;
     while(i < 24) {
         LPF_quarter_coefficients[i] = LPF_coefficients[i] * j;
         i++;
     }
     j = 1536 * volume / Max_volume;
     i = 27;
     while(i < 30) {
         LPF_quarter_coefficients[i] = LPF_coefficients[i] * j;
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
    executionState.pCoefficients = LPF_quarter_coefficients;    // Each call to filterBiquad() will advance pState and pCoefficients to the next biquad

  // The 1st call to filter1_filterBiquad() reads from the caller supplied input buffer and writes to the output buffer.
  // The remaining calls to filterBiquad() recycle the same output buffer, so that multiple intermediate buffers are not required.
  LPF_filterBiquad0( &executionState );   // Run biquad #0
  executionState.pInput = executionState.pOutput;         // The remaining biquads will now re-use the same output buffer.
  LPF_filterBiquad1( &executionState );   // Run biquad #1
  executionState.pInput = executionState.pOutput;
  LPF_filterBiquad2( &executionState );   // Run biquad #2
  executionState.pInput = executionState.pOutput;
  LPF_filterBiquad3( &executionState );   // Run biquad #3
  executionState.pInput = executionState.pOutput;
  LPF_filterBiquad4( &executionState );   // Run biquad #4


  // At this point, the caller-supplied output buffer will contain the filtered samples and the input buffer will contain the unmodified input samples.
  return count;   // Return the number of samples processed, the same as the number of input samples

}
//
//void LPF_filterBiquad12( LPF_executionState * pExecState )
//{
//   // Read state variables
//   int32_t w0, x0, w1[2], w2[2];
//   w1[0] = pExecState->pState[0];
//   w2[0] = pExecState->pState[1];
//   w1[1] = pExecState->pState[2];
//   w2[1] = pExecState->pState[3];
//   // Read coefficients into work registers
//   int16_t a0 = *(pExecState->pCoefficients++);
//   int16_t a1 = *(pExecState->pCoefficients++);
//   int16_t a2 = *(pExecState->pCoefficients++);
//   int16_t b0 = *(pExecState->pCoefficients++);
//   int16_t b1 = *(pExecState->pCoefficients++);
//   int16_t b2 = *(pExecState->pCoefficients++);
// 
//   // Read source and target pointers
//   int32_t *pInput  = pExecState->pInput;
//   int32_t *pOutput = pExecState->pOutput;
//   int32_t count = pExecState->count;
//   int32_t accumulator;
//   int16_t i = 0;
//   int16_t ch_sel;
//   ch_sel = (pExecState->nb_channel == 2)? 1: 0;
// // Loop for all samples in the input buffer
//      while( count-- )
//          {
//              //stereo or mono
//              i = count & ch_sel;
// 
//              // Read input sample
//              x0 = *(pInput++);
//              // Run feedback part of filter
//              accumulator  = a2 * ((w2[i])>>7);
//              accumulator += a1 * ((w1[i])>>7);
//              accumulator -= x0;
// 
//              w0 = accumulator;
// 
//              // Run feedforward part of filter
//              accumulator  = ((w0 * b0));
//              accumulator += ((w1[i] * b1));
//              accumulator += ((w2[i] * b2));
// 
//              w2[i] = w1[i]; // Shuffle history buffer
//              w1[i] = w0;
// 
//              // Write output
//              *(pOutput++) = accumulator >> 8;
//          }
// 
// 
//   // Write state variables
//   *(pExecState->pState++) = w1[0];
//   *(pExecState->pState++) = w2[0];
//   *(pExecState->pState++) = w1[1];
//   *(pExecState->pState++) = w2[1];
//}


void LPF_filterBiquad0( LPF_executionState * pExecState )
{
  // Read state variables
  int32_t w0, x0, w1[2], w2[2];
  w1[0] = pExecState->pState[0];
  w2[0] = pExecState->pState[1];
  w1[1] = pExecState->pState[2];
  w2[1] = pExecState->pState[3];
  // Read coefficients into work registers
  int16_t a0 = *(pExecState->pCoefficients++);
  int16_t a1 = *(pExecState->pCoefficients++);
  int16_t a2 = *(pExecState->pCoefficients++);
  int16_t b0 = *(pExecState->pCoefficients++);
  int16_t b1 = *(pExecState->pCoefficients++);
  int16_t b2 = *(pExecState->pCoefficients++);

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
             accumulator  = a2 * ((w2[i]));
             accumulator += a1 * ((w1[i]));
             accumulator -= x0 << 3;

             w0 = accumulator >> 7;

             // Run feedforward part of filter
             accumulator  = ((w0 * b0));
             accumulator += ((w1[i] * b1));
             accumulator += ((w2[i] * b2));

             w2[i] = w1[i]; // Shuffle history buffer
             w1[i] = w0;

             // Write output
             *(pOutput++) = accumulator >> 7;
         }


  // Write state variables
  *(pExecState->pState++) = w1[0];
  *(pExecState->pState++) = w2[0];
  *(pExecState->pState++) = w1[1];
  *(pExecState->pState++) = w2[1];

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
  int16_t a0 = *(pExecState->pCoefficients++);
  int16_t a1 = *(pExecState->pCoefficients++);
  int16_t a2 = *(pExecState->pCoefficients++);
  int16_t b0 = *(pExecState->pCoefficients++);
  int16_t b1 = *(pExecState->pCoefficients++);
  int16_t b2 = *(pExecState->pCoefficients++);

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
             accumulator  = a2 * ((w2[i]));
             accumulator += a1 * ((w1[i]));
             accumulator -= x0 << 3;

             w0 = accumulator >> 7;

             // Run feedforward part of filter
             accumulator  = ((w0 * b0));
             accumulator += ((w1[i] * b1));
             accumulator += ((w2[i] * b2));

             w2[i] = w1[i]; // Shuffle history buffer
             w1[i] = w0;

             // Write output
             *(pOutput++) = accumulator >> 7;
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
  int16_t a0 = *(pExecState->pCoefficients++);
  int16_t a1 = *(pExecState->pCoefficients++);
  int16_t a2 = *(pExecState->pCoefficients++);
  int16_t b0 = *(pExecState->pCoefficients++);
  int16_t b1 = *(pExecState->pCoefficients++);
  int16_t b2 = *(pExecState->pCoefficients++);

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
             accumulator  = a2 * ((w2[i]));
             accumulator += a1 * ((w1[i]));
             accumulator -= x0 << 5;

             w0 = accumulator >> 7;

             // Run feedforward part of filter
             accumulator  = ((w0 * b0));
             accumulator += ((w1[i] * b1));
             accumulator += ((w2[i] * b2));

             w2[i] = w1[i]; // Shuffle history buffer
             w1[i] = w0;

             // Write output
             *(pOutput++) = accumulator >> 7;
         }


  // Write state variables
  *(pExecState->pState++) = w1[0];
  *(pExecState->pState++) = w2[0];
  *(pExecState->pState++) = w1[1];
  *(pExecState->pState++) = w2[1];

}


void LPF_filterBiquad3( LPF_executionState * pExecState )
{
  // Read state variables
  int32_t w0, x0, w1[2], w2[2];
  w1[0] = pExecState->pState[0];
  w2[0] = pExecState->pState[1];
  w1[1] = pExecState->pState[2];
  w2[1] = pExecState->pState[3];
  // Read coefficients into work registers
  int16_t a0 = *(pExecState->pCoefficients++);
  int16_t a1 = *(pExecState->pCoefficients++);
  int16_t a2 = *(pExecState->pCoefficients++);
  int16_t b0 = *(pExecState->pCoefficients++);
  int16_t b1 = *(pExecState->pCoefficients++);
  int16_t b2 = *(pExecState->pCoefficients++);

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
             accumulator  = a2 * ((w2[i]));
             accumulator += a1 * ((w1[i]));
             accumulator -= x0 << 5;

             w0 = accumulator >> 7;

             // Run feedforward part of filter
             accumulator  = ((w0 * b0));
             accumulator += ((w1[i] * b1));
             accumulator += ((w2[i] * b2));

             w2[i] = w1[i]; // Shuffle history buffer
             w1[i] = w0;

             // Write output
             *(pOutput++) = accumulator >> 7;
         }


  // Write state variables
  *(pExecState->pState++) = w1[0];
  *(pExecState->pState++) = w2[0];
  *(pExecState->pState++) = w1[1];
  *(pExecState->pState++) = w2[1];

}

void LPF_filterBiquad4( LPF_executionState * pExecState )
{
  // Read state variables
  int32_t w0, x0, w1[2], w2[2];
  w1[0] = pExecState->pState[0];
  w2[0] = pExecState->pState[1];
  w1[1] = pExecState->pState[2];
  w2[1] = pExecState->pState[3];
  // Read coefficients into work registers
  int16_t a0 = *(pExecState->pCoefficients++);
  int16_t a1 = *(pExecState->pCoefficients++);
  int16_t a2 = *(pExecState->pCoefficients++);
  int16_t b0 = *(pExecState->pCoefficients++);
  int16_t b1 = *(pExecState->pCoefficients++);
  int16_t b2 = *(pExecState->pCoefficients++);

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
             accumulator  = a2 * ((w2[i]));
             accumulator += a1 * ((w1[i]));
             accumulator -= x0 << 3;

             w0 = accumulator >> 7;

             // Run feedforward part of filter
             accumulator  = ((w0 * b0));
             accumulator += ((w1[i] * b1));
             accumulator += ((w2[i] * b2));

             w2[i] = w1[i]; // Shuffle history buffer
             w1[i] = w0;

             // Write output
             *(pOutput++) = accumulator >> 7;
         }


  // Write state variables
  *(pExecState->pState++) = w1[0];
  *(pExecState->pState++) = w2[0];
  *(pExecState->pState++) = w1[1];
  *(pExecState->pState++) = w2[1];

}

void LPF_filterBiquad_float( LPF_executionState * pExecState )
{
  // Read state variables
int32_t w0, x0, w1[2], w2[2];
  w1[0] = pExecState->pState[0];
  w2[0] = pExecState->pState[1];
  w1[1] = pExecState->pState[2];
  w2[1] = pExecState->pState[3];
  // Read coefficients into work registers
  float a0 = *(pExecState->pCoefficients++);
  float a1 = *(pExecState->pCoefficients++);
  float a2 = *(pExecState->pCoefficients++);
  float b0 = *(pExecState->pCoefficients++);
  float b1 = *(pExecState->pCoefficients++);
  float b2 = *(pExecState->pCoefficients++);

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
             accumulator  = a2 * w2[i];
             accumulator += a1 * w1[i];
             accumulator -= x0;

             w0 = accumulator;

             // Run feedforward part of filter
             accumulator  = w0 * b0;
             accumulator += w1[i]* b1;
             accumulator += w2[i]* b2;

             w2[i] = w1[i]; // Shuffle history buffer
             w1[i] = w0;

             // Write output
             *(pOutput++) = accumulator;
         }


  // Write state variables
  *(pExecState->pState++) = w1[0];
  *(pExecState->pState++) = w2[0];
  *(pExecState->pState++) = w1[1];
  *(pExecState->pState++) = w2[1];

}

