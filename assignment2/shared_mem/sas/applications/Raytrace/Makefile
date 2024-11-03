MACROS = ../../macros/c.linux.m4
LIBS = -lm -lpthread
INCS = 
CPP = gcc
CFLAGS = -Wall -O2 -fno-inline 

OBJS = \
bbox.o \
cr.o \
env.o \
fbuf.o \
geo.o \
huprn.o \
husetup.o \
hutv.o \
isect.o \
main.o \
matrix.o \
memory.o \
poly.o \
raystack.o \
shade.o \
sph.o \
trace.o \
tri.o \
workpool.o
DEFS = \
rt.h

RAYTRACE: Raytrace

Raytrace: $(OBJS) $(DEFS)
	gcc -g $(INCS) -o $@ $(OBJS) $(LIBS) -Wl

%.o: %.c $(DEFS)
	$(CPP) -g $(INCS) -c $(CFLAGS) $< -o $@

%.c:  %.C4 $(MACROS)
	m4 $(MACROS) $< > $@

%.h:  %.H4 $(MACROS)
	m4 $(MACROS) $< > $@

clean:
	rm -rf Raytrace
	rm -rf *.o
