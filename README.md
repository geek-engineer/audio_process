Audio_up_down_sampling README
=============

* This project is for audio up and down sampling

* The core code is generated from http://www.micromodeler.com/dsp/#

* however, the variable type and coefficient are modified

## Compile
To compile the prject just type:

	shell: make all

## Run

To run the executable file just type 
	
    shell: ./audio_pro.exe file.wav
    
## Clean

	shell: make clean

## Test
 Open sine_tone folder and unzip sine_tone.rar
 make sure audio_resample.exe exist under current dir
 ,run up_down.bat, it will proceed all wav file defined
 ,you can use wavepad software to check the FFT
