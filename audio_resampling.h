#ifndef AUDIO_RESAMPLING_ // Include guards
#define AUDIO_RESAMPLING_

int down_half_sample(int16_t *in_data, int16_t *out_data, uint32_t length, uint8_t nb_channel);
int up_double_sample(int16_t *in_data, int16_t *out_data, uint32_t length, uint8_t nb_channel);

#endif // LPF_H_

