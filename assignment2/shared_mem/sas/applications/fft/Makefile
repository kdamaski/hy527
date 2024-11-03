MACROS = ../../macros/c.linux.m4
LIBS = -lm -lpthread 
INCS =
CC = gcc
CFLAGS = -Wall -fno-inline -O2 

fft:	fft.c 
	$(CC) $(CFLAGS) $(INCS) -o $@  $< $(LIBS)

fft.c:  fft.C4 $(MACROS)
	m4 $(MACROS) $< > $@

clean:
	rm -f fft fft.c

