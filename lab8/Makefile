calculator: main.c add.c sub.c mult.c div.c num.c
	gcc -c -fPIC add.c sub.c mult.c div.c
	gcc -shared -o libcalculator.so add.o sub.o mult.o div.o
	gcc main.c num.c -L. -lcalculator -o calculator

run: calculator
	LD_LIBRARY_PATH=.:$$LD_LIBRARY_PATH ./calculator

clean:
	rm -f add.o div.o mult.o sub.o libcalculator.so calculator

.PHONY: clean
