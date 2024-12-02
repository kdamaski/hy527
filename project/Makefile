default:
	echo "Targets: [thread-server, event-server, uv-server, clean]"

event-server:
	gcc libevent-server.c -levent -o event-server

uv-server:
	gcc libuv-server.c -luv -o uv-server

thread-server:
	gcc thread-server.c -o test_uthr # misc.c swtch_x86.s -z noexecstack 

clean:
	rm -f event-server uv-server test_uthr
