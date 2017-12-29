#include <stdlib.h> // For malloc/free
#include <string.h> // For memset
#include <stdio.h>	// For test case I/O
#include <time.h> 

#include "LPF.h"
int8_t WAV_PCM_header[44] =
{
	0x52, 0x49, 0x46, 0x46, 0x24, 0x35, 0x0c, 0x00, 0x57,0x41, 0x56, 0x45, 0x66, 0x6d, 0x74, 0x20
	,0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x22, 0x56, 0x00, 0x00, 0x88, 0x58, 0x01, 0x00
	,0x02, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74,0x61, 0x00,0x35, 0x0c, 0x00
};

int down_half_sample(int16_t *in_data, int16_t *out_data, uint32_t length)
{
	uint32_t samplesProcessed, i, j, frame_size;
	int16_t *filtered_dat;
	i = j= 0;

	//allocate buffer for filtered data
	filtered_dat = malloc(length*sizeof(int16_t));

	//lowpass filter
	LPFType *filter = LPF_create(); // Create an instance of the filter
	LPF_reset( filter );
	while(length > 0) {
		samplesProcessed = LPF_filterBlock( filter, in_data, filtered_dat, length);		// Filter the input test signal
	}
	while(i < length) {
		if((i%2) == 0) {
				out_data[j] = filtered_dat[i];
				j++;
			}
		i++;
	}

	LPF_destroy(filter);
	free(filtered_dat);
	return samplesProcessed;
}

int up_double_sample(int16_t *in_data, int16_t *out_data, uint32_t length)
{
	int samplesProcessed;
	int16_t *double_dat;
	int i, j;
	int16_t null_dat = 0x0000;
	i = j = 0;
	double_dat = malloc(2 * length * sizeof(int16_t));
	while(i < 2 * length) {
		if((i % 2) == 0) {
			double_dat[i] = in_data[j];
			j++;
		}else
			double_dat[i] = null_dat;
		i++;
	}

	LPFType *filter = LPF_create(); // Create an instance of the filter
	LPF_reset( filter );
	samplesProcessed = LPF_filterInChunks( filter, double_dat, out_data, 2 * length);		// Filter the input test signal


	LPF_destroy(filter);
	free(double_dat);

	return samplesProcessed;

}
int main(int argc, char *argv[])
{
	FILE *fd_in, *fd_out;
	int16_t *samp_dat, *down_dat, *up_dat;
	int samplesProcessed;
	uint32_t audio_len = 44100 * 9;
	uint32_t i = 0;
	char file[50];
	sprintf(file,"%s",argv[1]);

	/*-------------here is test code------------------*/
	 long            ms; // Milliseconds
    time_t          s;  // Seconds
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    s  = spec.tv_sec;
	printf("nano second:%d",spec.tv_nsec);

	/*---------------end of test----------------------*/
	fd_in = fopen(file, "r");
	if(!fd_in) {
		printf("file open fail!");
		return 0;
	}
	//wav pcm header
	//	i = 0;
	//	for(i = 0; i < 44; i++) {
	//		fwrite(&WAV_PCM_header[i], sizeof(int8_t), 1, fd_out);
	//	}

	fseek(fd_in, 44, SEEK_SET); //start of data


	samp_dat = malloc(audio_len*sizeof(int16_t));
	while(i < audio_len){
		fread(&samp_dat[i], sizeof(int16_t), 1, fd_in);
		//printf("read%d:%x \r\n",1024 - i, samp_dat[i]);
		i++;
	}

	//down sample process
	down_dat = malloc(audio_len / 2 *sizeof(int16_t)); //prepare buffer for output
	down_half_sample(samp_dat, down_dat, audio_len);


	//write down sample data to file
	char file_out[50];
	sprintf(file_out,"down_%s",file);
	fd_out = fopen(file_out, "w");
	if(!fd_out) {
		printf("file open fail!");
		return 0;
	}
	i = 20; //drop the first 20 data avoid noise
	while(i < audio_len / 2) {
			fwrite(&down_dat[i], sizeof(int16_t), 1, fd_out);
		i++;
	}

	//double up sample
	up_dat = malloc(audio_len *sizeof(int16_t)); //prepare buffer for output
	up_double_sample(down_dat, up_dat, audio_len/2);

	//write up sample data to file
	FILE *f_up;
	char file_up_out[50];
	sprintf(file_up_out,"up_%s",file);
	f_up = fopen(file_up_out, "w");
	i = 0;
	i = 20; //drop the first 20 data avoid noise
	while(i < audio_len)
	{
		fwrite(&up_dat[i], sizeof(int16_t), 1, f_up);
		i++;
	}


	fclose(fd_in);
	fclose(fd_out);
	fclose(f_up);
	free(samp_dat);
	free(down_dat);
	free(up_dat);
	return 1;
}

