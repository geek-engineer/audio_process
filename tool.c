#include <string.h> // For memset
#include <stdio.h>	// For test case I/O

#include "include.h"

void get_audio_header(char *filename)
{
    FILE *fd_in;
    //open audio file
    fd_in = fopen(filename, "rb");
	if(!fd_in) {
		printf("file open fail!");
	}

    //check "RIFF"
     fseek(fd_in, 0, SEEK_SET); fread(&wavFmt.RIFF, 4, 1, fd_in);
     if(wavFmt.RIFF != 0x46464952) { //FFIR
        printf("wavFmt.RIFF:0x%x \n",wavFmt.RIFF);
        printf("RIFF header missing!\n");
        goto exit;
     }

     //ChunkSize
     fread(&wavFmt.ChunkSize, 4, 1, fd_in);
     printf("Audio ChunkSize:%d \n", wavFmt.ChunkSize);

    //Format
    fread(&wavFmt.Format, 4, 1, fd_in);
    if(wavFmt.Format != 0x45564157) {
        printf("wavFmt.Format:0x%x \n", wavFmt.Format);
        printf("WAVE missing! \n");
        goto exit;
    }

    //SubChunk1ID
    fread(&wavFmt.SubChunk1ID, 4, 1, fd_in); //tmF
    if(wavFmt.SubChunk1ID != 0x20746d66) {
        printf("Fmt missing! \n");
        goto exit;
    }

    //SubChunk1Size
    fread(&wavFmt.SubChunk1Size, 4, 1, fd_in);
    printf("Audio SubChunk1Size:%d \n", wavFmt.SubChunk1Size);

    //Audio format
    fread(&wavFmt.AudioFomat, 2, 1, fd_in);
    printf("Audio AudioFomat:%d \n", wavFmt.AudioFomat);

    //NumChannel
    fread(&wavFmt.NumChannel, 2, 1, fd_in);
    printf("Audio Num of channel:%d \n", wavFmt.NumChannel);

    //Sample rate
    fread(&wavFmt.SampleRate, 4, 1, fd_in);
    printf("Audio SampleRate:%d \n", wavFmt.SampleRate);

    //ByteRate
    fread(&wavFmt.ByteRate, 4, 1, fd_in);
    printf("Audio ByteRate:%d \n", wavFmt.ByteRate);

    //Block Align
    fread(&wavFmt.BlockAlign, 2, 1, fd_in);
    printf("Audio BlockAlign:%d \n", wavFmt.BlockAlign);

    //BitPerSample
    fread(&wavFmt.BitPerSample, 2, 1, fd_in);
    printf("Audio BitPerSample:%d \n", wavFmt.BitPerSample);

    //extra parameter, this only for move fd_in pointer
    if(wavFmt.SubChunk1Size != 16) {
        fread(&wavFmt.extra, 2, 1, fd_in);
    }

    //SubChunk2ID
    fread(&wavFmt.SubChunk2ID, 4, 1, fd_in); //tmF
    if(wavFmt.SubChunk2ID != 0x61746164) {
        printf("data missing! \n");
        goto exit;
    }

    //SubChunk2Size
    fread(&wavFmt.SubChunk2Size, 4, 1, fd_in);
    printf("Audio SubChunk2Size:%d \n", wavFmt.SubChunk2Size);

exit:
    fclose(fd_in);
}

void write_audio_header(FILE *fd_out)
{
    wavFmt.SubChunk1Size = 16;
    fwrite(&wavFmt, 44, 1, fd_out);
}

void get_audio_chunk(FILE *Fpt, uint8_t *buf, uint32_t numByte)
{
    fread(audData, numByte, 1, Fpt);
}

void write_audio_chunk(FILE *Fpt, uint8_t *buf, uint32_t numByte)
{
    fwrite(audData, numByte, 1, Fpt);
}


