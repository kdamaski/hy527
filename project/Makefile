default:
	echo "Targets: [thread-server, pthread-server, clean]"

thread-server: thread-server.c
	gcc thread-server.c uthread.c misc.c swtch_x86.s -z noexecstack -o thread-server

pthread-server: pthread-server.c
	gcc pthread-server.c -o pthread-server

clean:
	rm -f thread-server pthread-server
