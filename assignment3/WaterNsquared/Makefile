MACROS = ../c.linux.m4
LIBS = -lm -lpthread
INCS =
CPP = gcc
CFLAGS = -Wall -fno-inline -O2 

OBJS= \
bndry.o \
cshift.o \
cnstnt.o \
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
touch.h \
water.h \
wwpot.h

LOCK ?= spinlock
ALLOWED_LOCKS = spinlock ticketlock mcslock
LOCK_IMPL = spinlock.c

WATER: WaterNsquared

WaterNsquared: $(OBJS) $(DEFS) 
	gcc -g $(INCS) ../$(LOCK_IMPL) -o $@ $(OBJS) $(LIBS) -W

%.o: %.c $(DEFS) check_inputs
	$(CPP) -g $(INCS) -c $(CFLAGS) $< -o $@

%.c:  %.C4 $(MACROS)
	m4 $(MACROS) $< > $@

%.h:  %.H4 $(MACROS)
	m4 $(MACROS) $< > $@

clean:
	rm -f WaterNsquared *.o

check_inputs:
	@valid_lock=0; \
	for n in $(ALLOWED_LOCKS); do \
	    if [ "$$n" = "$(LOCK)" ]; then \
				valid_lock=1; \
				LOCK_IMPL=$(LOCK).c \
				break; \
			fi; \
	done; \
	if [ $$valid_lock -eq 0 ]; then \
	    echo "Error: LOCK '$(LOCK)' is not recognized. Did you mean one of: $(ALLOWED_LOCKS)?"; \
	    exit 1; \
	fi; \
	echo "\nLock name : $(LOCK) is valid!\n"

