all: sender receiver

sender: sender_multicast.c
	gcc -o sender sender_multicast.c

receiver: receiver_multicast.c
	gcc -o receiver receiver_multicast.c

clean:
	rm -f sender receiver
.PHONY: clean
