audio_filter: LPF_4rd.o
	gcc audio_resampling.c -o audio_resampling LPF_4rd.o

LPF_4rd.o:
	gcc -c LPF_4rd.c

# program:
# 	gcc audio_filter.c -o audio_filter LPF_4rd.o

clean:
	-rm -f *.o
	-rm -f *.exe