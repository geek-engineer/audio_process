#ifndef _INCLUDE_H_
#define _INCLUDE_H_
#include <stdint.h>

#define readByte 2880
uint8_t audData[readByte];
uint8_t proData[readByte];

typedef struct format
{
    uint32_t RIFF;
    uint32_t ChunkSize;
    uint32_t Format;
    uint32_t SubChunk1ID;
    uint32_t SubChunk1Size;
    uint16_t AudioFomat;
    uint16_t NumChannel;
    uint32_t SampleRate;
    uint32_t ByteRate;
    uint16_t BlockAlign;
    uint16_t BitPerSample;
    uint32_t SubChunk2ID;
    uint32_t SubChunk2Size;
    uint16_t extra;
} wav_format;

wav_format wavFmt;

typedef struct
{
	char        procType[20];
	uint16_t     procCmd;
}ProcArg;

typedef enum {
    HELP,
    HALF_F,
    THIRD_F,
    QUARD_F,
    HALF_D,
    THIRD_D,
    QUARD_D,
    MAX,
} argcmd;

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
#endif
