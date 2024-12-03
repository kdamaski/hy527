default:
	echo "Targets: [thread-server, event-server, uv-server, clean]"

event-server:
	gcc libevent-server.c -levent -o event-server

uv-server:
	gcc libuv-server.c -luv -o uv-server

thread-server:
	gcc thread.c misc.c swtch_x86.s -z noexecstack -o thread-server

clean:
	rm -f event-server uv-server thread-server
