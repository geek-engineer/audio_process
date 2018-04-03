#ifndef _TOOL_H_
#define _TOOL_H_

#include <stdint.h>
void get_audio_header(char *filename);
void write_audio_header(FILE *fd_out, wav_format fmt);
void get_audio_chunk(FILE *Fpt, uint8_t *buf, uint32_t numByte);
void write_audio_chunk(FILE *Fpt, uint8_t *buf, uint32_t numByte);
#endif
