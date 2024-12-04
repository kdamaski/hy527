default:
	echo "Targets: [thread-server, event-server, uv-server, clean]"

event-server: libevent-server.c
	gcc libevent-server.c -levent -o event-server

uv-server: libuv-server.c
	gcc libuv-server.c -luv -o uv-server

thread-server: thread-server.c
	gcc thread-server.c uthread.c misc.c swtch_x86.s -z noexecstack -o thread-server

pthread-server: pthread-server.c
	gcc pthread-server.c misc.c swtch_x86.s -z noexecstack -o pthread-server

clean:
	rm -f event-server uv-server thread-server pthread-server
