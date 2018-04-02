all: tool.o audio_resampling.o LPF.o
	gcc main.c tool.o audio_resampling.o LPF.o -o audio_pro.exe

tool.o:
	gcc -c tool.c

audio_resampling.o:
	gcc -c audio_resampling.c

LPF.o:
	gcc -c LPF.c

# program:
# 	gcc audio_filter.c -o audio_filter LPF_4rd.o

clean:
	-rm -f *.o
	-rm -f *.exe