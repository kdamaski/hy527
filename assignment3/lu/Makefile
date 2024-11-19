MACROS = ../c.linux.m4
LIBS = -lm -lpthread 
INCS =  
CC = gcc
CFLAGS = -Wall -fno-inline -O2 

LOCK ?= spinlock
ALLOWED_LOCKS = spinlock ticketlock mcslock
LOCK_IMPL = spinlock.c

lu:	lu.c check_inputs
	$(CC) $(CFLAGS) ../$(LOCK_IMPL) $(INCS) -o $@  $< $(LIBS)

lu.c:  lu.C4 $(MACROS)
	m4 $(MACROS) $< > $@

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

clean:
	rm -f lu 
