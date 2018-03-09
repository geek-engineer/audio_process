set sine_tone=%~dp0\sine_tone

make clean
make

mv audio_resampling.exe sine_tone/audio_resampling.exe

IF EXIST sine_tone/audio_resampling.exe (
	cp %output%/audio_resampling.exe  %sine_tone%
) ELSE (
	echo error!!
)

pause