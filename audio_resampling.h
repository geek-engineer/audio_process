#ifndef AUDIO_RESAMPLING_ // Include guards
#define AUDIO_RESAMPLING_

typedef struct
{
	uint16_t nb_channel;	//number of channel. 1:mono, 2:stereo
	uint32_t sample_rate;	//number of samples per second
	uint8_t bit_depth;		//Bits Per Sample
	uint32_t NumData;       //number of byte of audio data
	uint32_t NumSamples;	//number of samples
} audio_info;

int down_half_sample(int32_t *in_data, int32_t *out_data, audio_info info);
int up_double_sample(int32_t *in_data, int32_t *out_data, audio_info info);

#endif // LPF_H_

