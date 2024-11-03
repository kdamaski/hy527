MACROS = ../../macros/c.linux.m4
LIBS = -lm -lpthread 
INCS =  
CC = gcc
CFLAGS = -Wall -fno-inline -O2 

lu:	lu.c 
	$(CC) $(CFLAGS) $(INCS) -o $@  $< $(LIBS)

lu.c:  lu.C4 $(MACROS)
	m4 $(MACROS) $< > $@

clean:
	rm -f lu 
