thread-server: thread-server.c uthread.c thread-server.h uthread.h misc.h misc.c
	gcc thread-server.c uthread.c misc.c swtch_x86.s -lpthread -z noexecstack -O3 -o thread-server

clean:
	rm -f thread-server
