MACROS = ../../macros/c.linux.m4
LIBS = -lm -lpthread
INCS = 
CPP = gcc
CFLAGS = -Wall -fno-inline -O2 


OBJS= \
bndry.o \
cnstnt.o \
cshift.o \
initia.o \
interf.o \
intraf.o \
kineti.o \
mdmain.o \
poteng.o \
predcor.o \
syscons.o \
water.o 
DEFS=  \
cnst.h \
fileio.h \
frcnst.h \
global.h \
mddata.h \
mdvar.h \
params.h \
randno.h \
split.h \
water.h \
wwpot.h
#touch.h 

WATER: WaterSpatial

WaterSpatial: $(OBJS) $(DEFS)
	gcc -g $(INCS) -o $@ $(OBJS) $(LIBS) -Wl

%.o: %.c $(DEFS)
	$(CPP) -g $(INCS) -c $(CFLAGS) $< -o $@

%.c:  %.C4 $(MACROS)
	m4 $(MACROS) $< > $@

%.h:  %.H4 $(MACROS)
	m4 $(MACROS) $< > $@

clean:
	rm -f WaterSpatial
	rm -f *.o
