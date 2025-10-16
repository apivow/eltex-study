all: chat_server chat_client

chat_server:
	gcc chat_server.c -o chat_server -lrt

chat_client:
	gcc chat_client.c -o chat_client -pthread -lncursesw -lrt

clean:
	rm -f chat_server chat_client

.PHONY: clean
