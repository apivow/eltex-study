all: server client

server: server_inet_dgram.c
	gcc -o server server_inet_dgram.c

client: client_inet_dgram.c
	gcc -o client client_inet_dgram.c

clean:
	rm -f server client
.PHONY: clean
