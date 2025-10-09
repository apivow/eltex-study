all: shop

shop: main.c shop.c shop.h
	gcc -o shop main.c shop.c

run: shop
	./shop

clean:
	rm shop

.PHONY: all run clean
