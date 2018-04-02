#ifndef LPF_H_ // Include guards
#define LPF_H_
#include <stdint.h>
static const int LPF_numStages = 2;
static const int LPF_coefficientLength = 10;
extern const float LPF_coefficients[2][36];
extern int16_t LPF_shift_coefficients[2][36];

#define Max_volume 1000

typedef struct
{
	int32_t state[100];
	uint8_t nb_channel;
	uint8_t bit_depth;
	int16_t output;
	int8_t type;
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

 void LPF_volume(uint16_t volume, int8_t filter_type);
 uint32_t LPF_filterBlock( LPFType * pThis, int32_t * pInput, int32_t * pOutput, uint32_t count );
 void LPF_filterBiquad_float( LPF_executionState * pExecState );
 void LPF_filterBiquad0( LPF_executionState * pExecState );
 void LPF_filterBiquad1( LPF_executionState * pExecState );
 void LPF_filterBiquad2( LPF_executionState * pExecState );
 void LPF_filterBiquad3( LPF_executionState * pExecState );
 void LPF_filterBiquad4( LPF_executionState * pExecState );
 void LPF_filterBiquad5( LPF_executionState * pExecState );
#endif // LPF_H_

