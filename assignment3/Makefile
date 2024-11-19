MACROS = ./c.linux.m4
LIBS = -lpthread 
CC = gcc
CFLAGS = -Wall -fno-inline -O2

LOCK ?= spinlock
ALLOWED_LOCKS = spinlock ticket_lock mcs_lock

M4MACROS = -DLOCKDEC=$(LOCK)_t -DLOCKINIT=$(LOCK)_init -DLOCK=$(LOCK)_lock -DUNLOCK=$(LOCK)_unlock

mmul:	mmul.c
	$(CC) $(CFLAGS) -o $@  $< $(LIBS)

mmul.c: matmul.C4 $(MACROS) check_inputs
	m4 $(MACROS) $(M4MACROS) $< > $@

check_inputs:
	@valid_lock=0; \
	for n in $(ALLOWED_LOCKS); do \
	    if [ "$$n" = "$(LOCK)" ]; then valid_lock=1; break; fi; \
	done; \
	if [ $$valid_lock -eq 0 ]; then \
	    echo "Error: LOCK '$(LOCK)' is not recognized. Did you mean one of: $(ALLOWED_LOCKS)?"; \
	    exit 1; \
	fi; \
	echo "\nLock name : $(LOCK) is valid!\n"

clean:
	rm -f mmul mmul.c a.out
