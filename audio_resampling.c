#include <stdlib.h> // For malloc/free
#include <string.h> // For memset
#include <stdio.h>	// For test case I/O
#include <time.h>

#include "audio_resampling.h"
#include "LPF.h"

int8_t WAV_PCM_header[44] =
{
	0x52, 0x49, 0x46, 0x46, //ChunkID        : "RIFF" in big endian
	0x24, 0x35, 0x0c, 0x00, //ChunkSize      : 36 + SubChunk2Size
	0x57, 0x41, 0x56, 0x45, //Format         : "WAVE" in big endian
	0x66, 0x6d, 0x74, 0x20, //SubChunk1ID    : "fmt"
	0x10, 0x00, 0x00, 0x00, //SubChunk1Size  : 16 for pcm
	0x01, 0x00,             //AudioFormat    : PCM = 1
	0x01, 0x00,             //Num of channel : Mono = 1, Stereo = 2, etc
	0x22, 0x56, 0x00, 0x00, //SampleRate     : 44100(default)
	0x88, 0x58, 0x01, 0x00, //ByteRate       : SampleRate * NumChannel * BitsPerSample/8
	0x02, 0x00,             //BlockAlign     : NumChannel * BitsPerSample/8
	0x10, 0x00,             //BitsPerSample  : 16 (default)
	0x64, 0x61, 0x74, 0x61, //SSubChunk2ID   : "data" in big endian
	0x00, 0x35, 0x0c, 0x00, //SubChunk2Size
};

int down_half_sample(int32_t *in_data, int32_t *out_data, audio_info info)
{
	uint32_t samplesProcessed, i, j, frame_size;
	int32_t *filtered_dat;
	frame_size = 1024;

	//allocate buffer for filtered data
	filtered_dat = malloc(info.NumSamples * info.nb_channel * sizeof(int32_t));

	//lowpass filter
	LPFType *filter = LPF_create(); // Create an instance of the filter
	LPF_reset( filter);

	filter->nb_channel = info.nb_channel;
	filter->bit_depth = info.bit_depth;
	i = 0;
	j = info.NumSamples * info.nb_channel;
	while(j > 0) {
		if(frame_size > j) frame_size = j;
		samplesProcessed = LPF_filterBlock( filter, in_data + i, filtered_dat + i, frame_size);		// Filter the input test signal
		i += frame_size;
		j -= frame_size;
	}

	i = j= 0;
	while(i < info.NumSamples * info.nb_channel) {
		if((i & info.nb_channel) == 0) {
				out_data[j] = filtered_dat[i];
				j++;
			}
		i++;
	}

	LPF_destroy(filter);
	free(filtered_dat);
	return 0;
}


int down_quarter_sample(int32_t *in_data, int32_t *out_data, audio_info info)
{
	uint32_t i, j, frame_size;
	int32_t *filtered_dat;
	frame_size = 1024;

    //allocate buffer for filtered data
    filtered_dat = malloc(info.NumSamples* info.nb_channel * sizeof(int32_t));

    //lowpass filter
    LPFType *filter = LPF_create(); // Create an instance of the filter
    LPF_reset( filter);

    filter->nb_channel = info.nb_channel;
    filter->bit_depth = info.bit_depth;

	i = 0;
	j = info.NumSamples;
	while(j > 0) {
		if(frame_size > j) frame_size = j;
		LPF_filterBlock( filter, in_data + i, filtered_dat + i, frame_size);
		i += frame_size;
		j -= frame_size;
	}

	i = j= 0;
	while(i < info.NumSamples) {
		if((i % 4) == 0) {
				out_data[j] = filtered_dat[i];
				j++;
			}
		i++;
	}

	return 0;
}

int up_double_sample(int32_t *in_data, int32_t *out_data, audio_info info)
{
	int samplesProcessed;
	int32_t *double_dat;
	int i, j;
	i = j = 0;
	double_dat = malloc(2 * info.NumSamples * info.nb_channel * sizeof(int32_t));
	while(i < 2 * info.NumSamples * info.nb_channel) {
		if((i & info.nb_channel) == 0) {
			double_dat[i] = in_data[j];
			j++;
		}else
			double_dat[i] = double_dat[i - 1];
		i++;
	}

	//lowpass filter
	int frame_size;
	LPFType *filter = LPF_create(); // Create an instance of the filter
	LPF_reset( filter );
	frame_size = 1024;
	filter->nb_channel = info.nb_channel;
	filter->bit_depth = info.bit_depth;

	i = 0;
	j = info.NumSamples * info.nb_channel * 2;
	while(j > 0) {
		if(frame_size > j) frame_size = j;
		samplesProcessed = LPF_filterBlock( filter, double_dat + i, out_data + i, frame_size);		// Filter the input test signal
		i += frame_size;
		j -= frame_size;
	}

	LPF_destroy(filter);
	free(double_dat);

	return 0;

}

int up_quadruple_sample(int32_t *in_data, int32_t *out_data, audio_info info)
{
    int i, j;
    int frame_size;
    int32_t *quadruple_dat;
    i = j = 0;
    
	quadruple_dat = malloc(4 * info.NumSamples * info.nb_channel * sizeof(int32_t));
    while(i < 4 * info.NumSamples) {
        if((i % 4 ) == 0) {
            quadruple_dat[i] = in_data[j];
            j++;
        }else
            quadruple_dat[i] = quadruple_dat[i - 1];
        i++;
    }
    frame_size = 1024;
    //lowpass filter
	LPFType *filter = LPF_create(); // Create an instance of the filter
	LPF_reset( filter);
    filter->nb_channel = info.nb_channel;
    filter->bit_depth = info.bit_depth;

    i = 0;
    j = info.NumSamples * 4;
    while(j > 0) {
        if(frame_size > j) frame_size = j;
        LPF_filterBlock( filter, quadruple_dat + i, out_data + i, frame_size);
        i += frame_size;
        j -= frame_size;
    }

    return 0;

}

void get_audio_header(FILE *fd_in, audio_info *audio)
{
    //initialize audio info
    fseek(fd_in, 4, SEEK_SET);
    fread(&WAV_PCM_header[4], 4, 1, fd_in);
//  printf("chunkSize:%x %x %x %x", WAV_PCM_header[4],WAV_PCM_header[5],WAV_PCM_header[6],WAV_PCM_header[7]);
    fseek(fd_in, 22, SEEK_SET); //NumChannel
    fread(&audio->nb_channel, 2, 1, fd_in);
    fread(&audio->sample_rate, 4, 1, fd_in);
    fseek(fd_in, 34, SEEK_SET); //BitPerSample
    fread(&audio->bit_depth, 2, 1, fd_in);
    fseek(fd_in, 42, SEEK_SET); //num of data byte
    fread(&audio->NumData, 4, 1, fd_in);
    printf("audio.nb_channel:%d\n audio.sample_rate:%d\n audio.bit_depth:%d\n audio.NumData:%d\n", audio->nb_channel, audio->sample_rate, audio->bit_depth, audio->NumData);
    audio->NumSamples = (audio->NumData / audio->nb_channel) / (audio->bit_depth / 8);
//  audio.NumSamples = audio.sample_rate * audio.nb_channel * 10;
}

void get_audio_chunk(FILE *fd_in, audio_info *audio, int32_t *samp_dat)
{
    uint32_t i = 0;

    //read data from file
    fseek(fd_in, 44, SEEK_SET); //start of data
    while(i < audio->NumSamples * audio->nb_channel){
        fread(&samp_dat[i], audio->bit_depth / 8, 1, fd_in);
        if(samp_dat[i] & 0x8000 && audio->bit_depth == 16)
            samp_dat[i] |= 0xFFFF0000;
        else if(samp_dat[i] & 0x800000 && audio->bit_depth == 24)
            samp_dat[i] |= 0xFF000000;
        //printf("read%d:%x \r\n",1024 - i, samp_dat[i]);
        i++;
    }
}

int up_down_sample_half(int argc, char *argv[])
{
	FILE *fd_in, *fd_out;
	int32_t *samp_dat, *down_dat, *up_dat;
	uint32_t i = 0;
	char file[50];
    audio_info audio;
	if(!argv[1])
	    printf("no input! \n");
	sprintf(file,"%s",argv[1]);

	fd_in = fopen(file, "r");
	if(!fd_in) {
		printf("file open fail! \n");
		return 0;
	}


    //init audio info
    get_audio_header(fd_in, &audio);

    //init filter coefficient
    LPF_volume(48, audio.bit_depth);

    //read audio data
    samp_dat = malloc(audio.NumSamples * audio.nb_channel * sizeof(int32_t));
    get_audio_chunk(fd_in, &audio, samp_dat);

	//down sample process
	down_dat = malloc(audio.NumSamples * audio.nb_channel * sizeof(int32_t)); //prepare buffer for output
	down_half_sample(samp_dat, down_dat, audio);


	//write down sample data to file
	char file_out[50];
	sprintf(file_out,"down_%s",file);
	fd_out = fopen(file_out, "w");
	if(!fd_out) {
		printf("file open fail!");
		return 0;
	}

    //wav pcm header
    uint32_t ChunkSize, SampleRate, NumDataByte;
    NumDataByte = audio.NumData / 2;
    memcpy(&WAV_PCM_header[40], &NumDataByte, 4);
    ChunkSize = 36 + NumDataByte;
    memcpy(&WAV_PCM_header[4], &ChunkSize, 4);
    memcpy(&WAV_PCM_header[22], &audio.nb_channel, 2);
    SampleRate = 44100 / 2;
    memcpy(&WAV_PCM_header[24], &SampleRate, 4);
    i = 0;
    for(i = 0; i < 44; i++) {
        fwrite(&WAV_PCM_header[i], sizeof(int8_t), 1, fd_out);
    }

	i = 0;
	while(i < audio.NumSamples) {
		fwrite(&down_dat[i], audio.bit_depth / 8, 1, fd_out);
		i++;
	}


	//double up sample
	LPF_volume(48, audio.bit_depth);
	up_dat = malloc((audio.NumSamples * audio.nb_channel) * sizeof(int32_t)); //prepare buffer for output
	audio.NumSamples = audio.NumSamples / 2;
	up_double_sample(down_dat, up_dat, audio);

    //write up sample data to file
    FILE *f_up;
    char file_up_out[50];
    sprintf(file_up_out,"up_%s",file);
    f_up = fopen(file_up_out, "w");
    if(!f_up) {
        printf("file open fail!");
        return 0;
    }

    //wav pcm header
    NumDataByte = audio.NumData;
    memcpy(&WAV_PCM_header[40], &NumDataByte, 4);
    ChunkSize = 36 + NumDataByte;
    memcpy(&WAV_PCM_header[4], &ChunkSize, 4);
    SampleRate = 44100;
    memcpy(&WAV_PCM_header[24], &SampleRate, 4);
    i = 0;
    for(i = 0; i < 44; i++) {
        fwrite(&WAV_PCM_header[i], sizeof(int8_t), 1, f_up);
    }

    i = 0;
	while(i < audio.NumSamples * 2)
	{
		fwrite(&up_dat[i], audio.bit_depth / 8, 1, f_up);
		i++;
	}

	fclose(fd_in);
	fclose(fd_out);
	fclose(f_up);
	free(samp_dat);
	free(down_dat);
	free(up_dat);
	return 0;
}


int up_down_sample_quarter(int argc, char *argv[])
{
	FILE *fd_in, *fd_out;
	int32_t *samp_dat, *down_dat, *up_dat;
	uint32_t i = 0;
	char file[50];
    audio_info audio;
	if(!argv[1])
	    printf("no input! \n");
	sprintf(file,"%s",argv[1]);

	fd_in = fopen(file, "r");
	if(!fd_in) {
		printf("file open fail! \n");
		return 0;
	}


    //init audio info
    get_audio_header(fd_in, &audio);

    //init filter coefficient
//    LPF_volume(60, audio.bit_depth);

    //read audio data
    samp_dat = malloc(audio.NumSamples * audio.nb_channel * sizeof(int32_t));
    get_audio_chunk(fd_in, &audio, samp_dat);

	//down sample process
	down_dat = malloc(audio.NumSamples * audio.nb_channel * sizeof(int32_t) / 4); //prepare buffer for output

	down_quarter_sample(samp_dat, down_dat, audio);


	//write down sample data to file
//	char file_out[50];
//	sprintf(file_out,"down_%s",file);
//	fd_out = fopen(file_out, "w");
//	if(!fd_out) {
//		printf("file open fail!");
//		return 0;
//	}

    //wav pcm header
    uint32_t ChunkSize, SampleRate, NumDataByte;
    NumDataByte = audio.NumData / 4;
    memcpy(&WAV_PCM_header[40], &NumDataByte, 4);
    ChunkSize = 36 + NumDataByte;
    memcpy(&WAV_PCM_header[4], &ChunkSize, 4);
    memcpy(&WAV_PCM_header[22], &audio.nb_channel, 2);
    SampleRate = 44100 / 4;
    memcpy(&WAV_PCM_header[24], &SampleRate, 4);
    i = 0;
//    for(i = 0; i < 44; i++) {
//        fwrite(&WAV_PCM_header[i], sizeof(int8_t), 1, fd_out);
//    }
//
//	i = 0;
//	while(i < audio.NumSamples * audio.nb_channel) {
//		fwrite(&down_dat[i], audio.bit_depth / 8, 1, fd_out);
//		i++;
//	}
	
	//double up sample
//	LPF_volume(100, audio.bit_depth);
	up_dat = malloc((audio.NumSamples * audio.nb_channel) * sizeof(int32_t)); //prepare buffer for output

	audio.NumSamples = audio.NumSamples / 4;
	up_quadruple_sample(down_dat, up_dat, audio);

    //write up sample data to file
    FILE *f_up;
    char file_up_out[50];
    sprintf(file_up_out,"up_%s",file);
    f_up = fopen(file_up_out, "w");
    if(!f_up) {
        printf("file open fail!");
        return 0;
    }

    //wav pcm header
    NumDataByte = audio.NumData;
    memcpy(&WAV_PCM_header[40], &NumDataByte, 4);
    ChunkSize = 36 + NumDataByte;
    memcpy(&WAV_PCM_header[4], &ChunkSize, 4);
    SampleRate = 44100;
    memcpy(&WAV_PCM_header[24], &SampleRate, 4);
    i = 0;
    for(i = 0; i < 44; i++) {
        fwrite(&WAV_PCM_header[i], sizeof(int8_t), 1, f_up);
    }

    i = 0;
	while(i < audio.NumSamples * audio.nb_channel * 4)
	{
		fwrite(&up_dat[i], audio.bit_depth / 8, 1, f_up);
		i++;
	}

	fclose(fd_in);
//	fclose(fd_out);
	fclose(f_up);
	free(samp_dat);
	free(down_dat);
	free(up_dat);
	return 0;
}

int up_down_drop_half(int argc, char *argv[])
{
	FILE *fd_in, *fd_out;
	int32_t *samp_dat, *down_dat, *up_dat;
	uint32_t i, j;
	char file[50];
    audio_info audio;
	if(!argv[1])
	    printf("no input! \n");
	sprintf(file,"%s",argv[1]);

	fd_in = fopen(file, "r");
	if(!fd_in) {
		printf("file open fail! \n");
		return 0;
	}


    //init audio info
    get_audio_header(fd_in, &audio);

    //read audio data
    samp_dat = malloc(audio.NumSamples * audio.nb_channel * sizeof(int32_t));
    get_audio_chunk(fd_in, &audio, samp_dat);

	//down sample process
	down_dat = malloc(audio.NumSamples * audio.nb_channel * sizeof(int32_t)); //prepare buffer for output
   i = j = 0;
   while (i < audio.NumSamples) {
        if (i & audio.nb_channel) {
            down_dat[j] = samp_dat[i];
            j++;
        }
        i++;
   }


	//write down sample data to file
	char file_out[50];
	sprintf(file_out,"down_%s",file);
	fd_out = fopen(file_out, "w");
	if(!fd_out) {
		printf("file open fail!");
		return 0;
	}

    //wav pcm header
    uint32_t ChunkSize, SampleRate, NumDataByte;
    NumDataByte = audio.NumData / 2;
    memcpy(&WAV_PCM_header[40], &NumDataByte, 4);
    ChunkSize = 36 + NumDataByte;
    memcpy(&WAV_PCM_header[4], &ChunkSize, 4);
    SampleRate = 44100 / 2;
    memcpy(&WAV_PCM_header[24], &SampleRate, 4);
    i = 0;
    for(i = 0; i < 44; i++) {
        fwrite(&WAV_PCM_header[i], sizeof(int8_t), 1, fd_out);
    }

	i = 0;
	while(i < audio.NumSamples) {
		fwrite(&down_dat[i], audio.bit_depth / 8, 1, fd_out);
		i++;
	}


	//double up sample
	up_dat = malloc((audio.NumSamples * audio.nb_channel) * sizeof(int32_t)); //prepare buffer for output
	audio.NumSamples = audio.NumSamples / 2;
    i = j = 0;
    while (i < audio.NumSamples) {
        up_dat[2 * i] = down_dat[i];
        up_dat[2 * i + 1] = up_dat[2 * i];
        i++;
    }

    //write up sample data to file
    FILE *f_up;
    char file_up_out[50];
    sprintf(file_up_out,"up_%s",file);
    f_up = fopen(file_up_out, "w");
    if(!f_up) {
        printf("file open fail!");
        return 0;
    }

    //wav pcm header
    NumDataByte = audio.NumData;
    memcpy(&WAV_PCM_header[40], &NumDataByte, 4);
    ChunkSize = 36 + NumDataByte;
    memcpy(&WAV_PCM_header[4], &ChunkSize, 4);
    SampleRate = 44100;
    memcpy(&WAV_PCM_header[24], &SampleRate, 4);
    i = 0;
    for(i = 0; i < 44; i++) {
        fwrite(&WAV_PCM_header[i], sizeof(int8_t), 1, f_up);
    }

    i = 0;
	while(i < audio.NumSamples * 2)
	{
		fwrite(&up_dat[i], audio.bit_depth / 8, 1, f_up);
		i++;
	}

	fclose(fd_in);
	fclose(fd_out);
	fclose(f_up);
	free(samp_dat);
	free(down_dat);
	free(up_dat);
	return 0;
}

int up_down_drop_quater(int argc, char *argv[])
{
	FILE *fd_in, *fd_out;
	int32_t *samp_dat, *down_dat, *up_dat;
	uint32_t i, j;
	char file[50];
    audio_info audio;
	if(!argv[1])
	    printf("no input! \n");
	sprintf(file,"%s",argv[1]);

	fd_in = fopen(file, "r");
	if(!fd_in) {
		printf("file open fail! \n");
		return 0;
	}


    //init audio info
    get_audio_header(fd_in, &audio);

    //read audio data
    samp_dat = malloc(audio.NumSamples * audio.nb_channel * sizeof(int32_t));
    get_audio_chunk(fd_in, &audio, samp_dat);

	//down sample process
	down_dat = malloc(audio.NumSamples * audio.nb_channel * sizeof(int32_t)); //prepare buffer for output
   i = j = 0;
   while (i < audio.NumSamples) {
        if (i%4 == 0) {
            down_dat[j] = samp_dat[i];
            j++;
        }
        i++;
   }


	//write down sample data to file
	char file_out[50];
	sprintf(file_out,"down_%s",file);
	fd_out = fopen(file_out, "w");
	if(!fd_out) {
		printf("file open fail!");
		return 0;
	}

    //wav pcm header
    uint32_t ChunkSize, SampleRate, NumDataByte;
    NumDataByte = audio.NumData / 4;
    memcpy(&WAV_PCM_header[40], &NumDataByte, 4);
    ChunkSize = 36 + NumDataByte;
    memcpy(&WAV_PCM_header[4], &ChunkSize, 4);
    SampleRate = 44100 / 4;
    memcpy(&WAV_PCM_header[24], &SampleRate, 4);
    i = 0;
    for(i = 0; i < 44; i++) {
        fwrite(&WAV_PCM_header[i], sizeof(int8_t), 1, fd_out);
    }

	i = 0;
	while(i < audio.NumSamples / 2) {
		fwrite(&down_dat[i], audio.bit_depth / 8, 1, fd_out);
		i++;
	}


	//double up sample
	up_dat = malloc((audio.NumSamples * audio.nb_channel) * sizeof(int32_t)); //prepare buffer for output
	audio.NumSamples = audio.NumSamples / 2;
    i = j = 0;
    while (i < audio.NumSamples / 2) {
        up_dat[4 * i] = down_dat[i];
        up_dat[4 * i + 1] = up_dat[4 * i];
        up_dat[4 * i + 2] = up_dat[4 * i];
        up_dat[4 * i + 3] = up_dat[4 * i];
        i++;
    }

    //write up sample data to file
    FILE *f_up;
    char file_up_out[50];
    sprintf(file_up_out,"up_%s",file);
    f_up = fopen(file_up_out, "w");
    if(!f_up) {
        printf("file open fail!");
        return 0;
    }

    //wav pcm header
    NumDataByte = audio.NumData;
    memcpy(&WAV_PCM_header[40], &NumDataByte, 4);
    ChunkSize = 36 + NumDataByte;
    memcpy(&WAV_PCM_header[4], &ChunkSize, 4);
    SampleRate = 44100;
    memcpy(&WAV_PCM_header[24], &SampleRate, 4);
    i = 0;
    for(i = 0; i < 44; i++) {
        fwrite(&WAV_PCM_header[i], sizeof(int8_t), 1, f_up);
    }

    i = 0;
	while(i < audio.NumSamples * 2)
	{
		fwrite(&up_dat[i], audio.bit_depth / 8, 1, f_up);
		i++;
	}

	fclose(fd_in);
	fclose(fd_out);
	fclose(f_up);
	free(samp_dat);
	free(down_dat);
	free(up_dat);
	return 0;
}

int main(int argc, char *argv[])
{
//    up_down_sample_half(argc, argv);
    up_down_sample_quarter(argc, argv);
//    up_down_drop_half(argc, argv);
//    up_down_drop_quater(argc, argv);
    return 0;
}

