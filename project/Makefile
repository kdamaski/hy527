all: pthread-server thread-server

thread-server: thread-server.c uthread.c thread-server.h uthread.h misc.h misc.c
	gcc thread-server.c uthread.c misc.c swtch_x86.s -lpthread -z noexecstack -O3 -o thread-server

pthread-server: pthread-server.c pthread-server.h
	gcc pthread-server.c -o pthread-server -lpthread -O3

clean:
	rm -f thread-server pthread-server
