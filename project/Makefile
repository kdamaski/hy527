default:
	echo "Targets: [threads, event-server, uv-server, clean]"

event-server:
	gcc libevent-server.c -levent -o event-server

uv-server:
	gcc libuv-server.c -luv -o uv-server

threads:
	gcc thread.c misc.c swtch_x86.s -z noexecstack -o test_uthr

clean:
	rm -f event-server uv-server test_uthr
