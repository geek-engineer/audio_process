@echo off
set sine_tone=%~dp0\sine_tone

make clean
make

IF EXIST audio_resampling.exe (
	mv audio_resampling.exe sine_tone/audio_resampling.exe
) ELSE (
	echo error!!
)

pause