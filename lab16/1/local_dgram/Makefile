all: server client

server: server_local_dgram.c
	gcc -o server server_local_dgram.c

client: client_local_dgram.c
	gcc -o client client_local_dgram.c

clean:
	rm -f server client
.PHONY: clean
