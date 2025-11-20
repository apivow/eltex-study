all: server client

server: server_local_stream.c
	gcc -o server server_local_stream.c

client: client_local_stream.c
	gcc -o client client_local_stream.c

clean:
	rm -f server client
.PHONY: clean
