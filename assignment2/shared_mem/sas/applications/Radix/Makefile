MACROS = ../../macros/c.linux.m4
LIBS = -lm -lpthread
INCS = 
CC = gcc
CFLAGS = -Wall -O2 -fno-inline 

radix:	radix.c 
	$(CC) $(CFLAGS) $(INCS) -o $@  $< $(LIBS)

radix.c: radix.C4 $(MACROS)
	m4 $(MACROS) $< > $@

clean:
	rm -f radix
	rm -f *.o
