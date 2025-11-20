all: server client

server: server_inet_stream.c
	gcc -o server server_inet_stream.c

client: client_inet_stream.c
	gcc -o client client_inet_stream.c

clean:
	rm -f server client
.PHONY: clean
