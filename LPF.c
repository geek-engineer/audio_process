#include "LPF.h"

#include <stdlib.h> // For malloc/free
#include <string.h> // For memset
#include <stdio.h>    // For test case I/O
const float LPF_coefficients[2][36] =
 {
    {
        //Sect 0
        -1.000000000000000000,0.838050133845431478,-0.000000000000000000,0.080278749293000995,0.080278749293000995,0.000000000000000000,
        //Sect 1
        -1.000000000000000000,1.657595652928926630,-0.731460773047021617,0.111682710659617379,-0.149135358210711750,0.111682710659617379,
        //Sect 2
        -1.000000000000000000,1.615386026953850700,-0.799674122171146862,0.254917911495761762,-0.326132148762933339,0.254917911495761762,
        //Sect 3
        -1.000000000000000000,1.574695577159638700,-0.872797831900672283,0.334343477777298892,-0.373147641564758270,0.344343477777298892,
        //Sect 4
        -1.000000000000000000,1.551814971195027320,-0.932455878400803684,0.287449355705251097,-0.197530372852341846,0.287449355705251097,
        //Sect 5
        -1.000000000000000000,1.553738838433933900,-0.978765397406591964,0.170235308824484288,0.080901766049915988,0.180235308824484288
    }, {
            //Sect 0
            -1.000000000000000000,1.442057469797411120,-0.528411212954901566,0.109270553351304464,-0.134347101945947123,0.109270553351304464,
            //Sect 1
            -1.000000000000000000,1.459955369517176080,-0.596878591469297115,0.151313045432695498,-0.169127368299195169,0.151313045432695498,
            //Sect 2
            -1.000000000000000000,1.492894076262163860,-0.705664020039408535,0.174259501532445765,-0.141070512664366987,0.174259501532445765,
            //Sect 3
            -1.000000000000000000,1.540130303157246190,-0.824690073529609102,0.141048570987286487,-0.004654314550280140,0.141048570987286487,
            //Sect 4
            -1.000000000000000000,1.607626211174523870,-0.942085635349023676,0.091919494914015659,0.142255482979030040,0.091919494914015659
    },
 };

int16_t LPF_shift_coefficients[2][36] =
{
    {
        -127,  108,    0,   10,   10,    0,
        -127,  212,  -94,   14,  -19,   14,
        -127,  207, -102,   33,  -42,   33,
        -127,  202, -112,   43,  -48,   43,
        -127,  198, -119,   37,  -25,   37,
        -127,  199, -125,   22,   10,   22
    }, {
        -82,  185,  -68,   14,  -17,   14,
        -150,  187,  -76,   19,  -22,   19,
        -150,  191,  -90,   22,  -18,   22,
        -150,  197, -106,   18,   -1,   18,
        -150,  206, -121,   12,   18,   12,
           0,    0,    0,    0,    0,    0,
    },
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

 void LPF_volume(uint16_t volume, int8_t filter_type)
 {
     //init the LPF coefficients
     int i, j;

 //    if(volume >= Max_volume) volume = Max_volume;

     j = 256 * volume / Max_volume;
     i = 3;
     while(i < 6) {
         LPF_shift_coefficients[filter_type][i] = LPF_coefficients[filter_type][i] * j;
         i++;
     }

     j = 512 * volume / Max_volume;
     i = 9;
     while(i < 12) {
         LPF_shift_coefficients[filter_type][i] = LPF_coefficients[filter_type][i] * j;
         i++;
     }
     j = 512 * volume / Max_volume;
     i = 15;
     while(i < 18) {
         LPF_shift_coefficients[filter_type][i] = LPF_coefficients[filter_type][i] * j;
         i++;
     }
     j = 256* volume / Max_volume;
     i = 21;
     while(i < 24) {
         LPF_shift_coefficients[filter_type][i] = LPF_coefficients[filter_type][i] * j;
         i++;
     }
     j = 256 * volume / Max_volume;
     i = 27;
     while(i < 30) {
         LPF_shift_coefficients[filter_type][i] = LPF_coefficients[filter_type][i] * j;
         i++;
     }
     j = 256 * volume / Max_volume;
     i = 33;
     while(i < 36) {
         LPF_shift_coefficients[filter_type][i] = LPF_coefficients[filter_type][i] * j;
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
    executionState.pCoefficients = LPF_shift_coefficients[pThis->type];    // Each call to filterBiquad() will advance pState and pCoefficients to the next biquad

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
    if(!pThis->type) {
      executionState.pInput = executionState.pOutput;
      LPF_filterBiquad5( &executionState );   // Run biquad #5
    }
  // At this point, the caller-supplied output buffer will contain the filtered samples and the input buffer will contain the unmodified input samples.
  return count;   // Return the number of samples processed, the same as the number of input samples

}

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
             accumulator += (x0 * a0) >> 2;

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
             accumulator += (x0 * a0) >> 2;

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
             accumulator += (x0 * a0) >> 2;

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
             accumulator += (x0 * a0) >> 1;

             w0 = accumulator >> 7;

             // Run feedforward part of filter
             accumulator  = ((w0 * b0));
             accumulator += ((w1[i] * b1));
             accumulator += ((w2[i] * b2));

             w2[i] = w1[i]; // Shuffle history buffer
             w1[i] = w0;

             // Write output
             *(pOutput++) = accumulator >> 6;
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
             accumulator += (x0 * a0) >> 1;

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


void LPF_filterBiquad5( LPF_executionState * pExecState )
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
             accumulator += (x0 * a0) >> 2;

             w0 = accumulator >> 7;

             // Run feedforward part of filter
             accumulator  = ((w0 * b0));
             accumulator += ((w1[i] * b1));
             accumulator += ((w2[i] * b2));

             w2[i] = w1[i]; // Shuffle history buffer
             w1[i] = w0;

             // Write output
             *(pOutput++) = accumulator >> 6;
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

