all: server client

server: server.c
	gcc -o server server.c -pthread -lrt

client: client.c
	gcc -o client client.c -pthread -lrt -lncurses

.PHONY: clean
clean:
	rm -f server client
