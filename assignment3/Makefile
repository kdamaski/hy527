MACROS = ./c.linux.m4
LIBS = -lpthread 
CC = gcc
CFLAGS = -Wall -fno-inline -O2

mmul:	mmul.c 
	$(CC) $(CFLAGS) -o $@  $< $(LIBS)

mmul.c: matmul.C4 $(MACROS)
	m4 $(MACROS) $< > $@

clean:
	rm -f mmul mmul.c a.out
