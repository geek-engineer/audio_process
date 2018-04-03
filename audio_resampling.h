#ifndef AUDIO_RESAMPLING_ // Include guards
#define AUDIO_RESAMPLING_
#include "stdint.h"
#include "LPF.h"
LPFType filter;

//typedef struct
//{
//	char        procType[20];
//	uint16_t     procCmd;
//}ProcArg;
//
//typedef enum {
//    HELP,
//    HALF_F,
//    THIRD_F,
//    QUARD_F,
//    HALF_D,
//    THIRD_D,
//    QUARD_D,
//    MAX,
//} argcmd;
//
//ProcArg WAVPROCARG[] =
//{
//    {"HALF_F"  , HALF_F     },
//    {"THIRD_F" , THIRD_F    },
//    {"QUARD_F" , QUARD_F    },
//    {"HALF_D"  , HALF_D     },
//    {"THIRD_D" , THIRD_D    },
//    {"QUARD_D" , QUARD_D    },
//    {"MAX"     , MAX       },
//};

typedef struct
{
	uint16_t nb_channel;	//number of channel. 1:mono, 2:stereo
	uint32_t sample_rate;	//number of samples per second
	uint8_t bit_depth;		//Bits Per Sample
	uint32_t subchunk1size;       //number of byte of audio data
	uint32_t NumData;       //number of byte of audio data
	uint32_t NumSamples;	//number of samples
	int8_t  filter_type;
} audio_info;

int down_half_sample(int32_t *in_data, int32_t *out_data, audio_info info);
int up_double_sample(int32_t *in_data, int32_t *out_data, audio_info info);
int down_sample_quarter(uint8_t *Data_i, uint8_t *Data_o, uint32_t numByteIn);
int up_down_sample(char *filename, uint16_t prcType);


#endif // LPF_H_

