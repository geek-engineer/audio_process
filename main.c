#include <stdlib.h> // For malloc/free
#include <string.h> // For memset
#include <stdio.h>	// For test case I/O
#include "include.h"
#include "tool.h"
#include "audio_resampling.h"


void show_info(void)
{
    printf(
        "\n Argument must be one of follow:"
        "\n HALF_F 	: up_down sample half with filter"
        "\n THIRD_F 	: up_down sample one third with filter"
        "\n QUARD_F 	: up_down sample one forth with filter"
        "\n HALF_D 	: up_down sample half with drop dot"
        "\n QUARD_D 	: up_down sample one forth with drop dot"
        "\n\n ex: audio_resampling.exe filename.wav QUARD_F"
        "\n"
    );
}


int (*audio_process[])(char *filename, uint16_t prcType) = {up_down_sample,};

int main(int argc, char * argv [ ])
{
    //make sure the argc is more than 1
    if(argc == 1) {show_info(); goto exit;}

    char filename[50];
    sprintf(filename, "%s", argv[1]);

    /*----get audio info----*/
    get_audio_header(filename);

    /*----process audio data----*/
    audio_process[0](filename, QUARD_F);

exit:
    return 0;
}

