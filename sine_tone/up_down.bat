@echo off
If EXIST audio_resampling.exe (
	IF EXIST sweep20-20000.wav (
		start audio_resampling.exe sweep20-20000.wav
		echo Process done!
	) ELSE (
		echo Warnimg!! audio file missing!!
	)
) ELSE (
	echo Warnimg!! File audio_resampling.exe is missing!!
)
pause