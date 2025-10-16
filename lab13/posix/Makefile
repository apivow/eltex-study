all: posix_server posix_client

posix_server:
	gcc posix_server.c -o posix_server

posix_client:
	gcc posix_client.c -o posix_client

clean:
	rm -f posix_server posix_client

.PHONY: clean
