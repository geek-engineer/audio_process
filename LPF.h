#ifndef LPF_H_ // Include guards
#define LPF_H_
#include <stdint.h>
static const int LPF_numStages = 2;
static const int LPF_coefficientLength = 10;
extern float LPF_coefficients[30];
extern int16_t LPF_quarter_coefficients[30];

#define Max_volume 1000

typedef struct
{
	int32_t state[100];
	uint8_t nb_channel;
	uint8_t bit_depth;
	int16_t output;
} LPFType;

typedef enum
{
	mono,
	stereo
} audio_channel;

typedef struct
{
	int32_t *pInput;
	int32_t *pOutput;
	int32_t *pState;
	int16_t *pCoefficients;
	uint16_t nb_channel;
	uint8_t coefficient_shift;
	uint32_t count;
} LPF_executionState;


 LPFType *LPF_create( void );
 void LPF_destroy( LPFType *pObject );
 void LPF_init( LPFType * pThis );
 void LPF_reset( LPFType * pThis );

 void LPF_volume(uint16_t volume, uint16_t bit_depth);
 uint32_t LPF_filterBlock( LPFType * pThis, int32_t * pInput, int32_t * pOutput, uint32_t count );
 void LPF_filterBiquad0( LPF_executionState * pExecState );
 void LPF_filterBiquad1( LPF_executionState * pExecState );
 void LPF_filterBiquad2( LPF_executionState * pExecState );
 void LPF_filterBiquad3( LPF_executionState * pExecState );
 void LPF_filterBiquad4( LPF_executionState * pExecState );
#endif // LPF_H_

