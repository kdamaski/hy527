# all: pthread-server thread-server
test-server32: test-server.c uthread32.c thread-server.h uthread.h
	gcc test-server.c uthread32.c swtch_x86_32.s -lpthread -z noexecstack -O3 -o test-server32 -m32 -g

test-server: test-server.c uthread.c misc.c thread-server.h uthread.h
	gcc test-server.c uthread.c misc.c swtch_x86.s -lpthread -z noexecstack -O3 -o test-server -g

thread-server32: thread-server.c uthread32.c thread-server.h uthread.h
	gcc thread-server.c uthread32.c swtch_x86_32.s -lpthread -z noexecstack -O3 -o thread-server32 -m32

thread-server: thread-server.c uthread.c thread-server.h uthread.h misc.h misc.c
	gcc thread-server.c uthread.c misc.c swtch_x86.s -lpthread -z noexecstack -O3 -o thread-server

pthread-server: pthread-server.c pthread-server.h
	gcc pthread-server.c -o pthread-server -lpthread -O3

clean:
	rm -f thread-server pthread-server thread-server32 test-server test-server32
