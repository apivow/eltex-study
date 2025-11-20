all: sender receiver

sender: sender_broadcast.c
	gcc -o sender sender_broadcast.c

receiver: receiver_broadcast.c
	gcc -o receiver receiver_broadcast.c

clean:
	rm -f sender receiver
.PHONY: clean
