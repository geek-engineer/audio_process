#ifndef LPF_H_ // Include guards
#define LPF_H_
#include <stdint.h>
static const int LPF_numStages = 2;
static const int LPF_coefficientLength = 10;
extern int16_t LPF_coefficients[10];

typedef struct
{
	int16_t state[8];
	int16_t output;
} LPFType;

typedef struct
{
	int16_t *pInput;
	int16_t *pOutput;
	int16_t *pState;
	int16_t *pCoefficients;
	uint32_t count;
} LPF_executionState;


 LPFType *LPF_create( void );
 void LPF_destroy( LPFType *pObject );
 void LPF_init( LPFType * pThis );
 void LPF_reset( LPFType * pThis );

 uint32_t LPF_filterBlock( LPFType * pThis, int16_t * pInput, int16_t * pOutput, uint32_t count );
 void LPF_filterBiquad( LPF_executionState * pExecState );
 int LPF_filterInChunks( LPFType * pThis, int16_t * pInput, int16_t * pOutput, uint32_t length );
#endif // LPF_H_
	

