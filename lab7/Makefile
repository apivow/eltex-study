calculator: main.c add.c sub.c mult.c div.c num.c
	gcc -c add.c sub.c mult.c div.c
	ar rcs libcalculator.a add.o sub.o mult.o div.o
	gcc main.c num.c -L. -lcalculator -o calculator

clean:
	rm -f *.o calculator.a calculator

.PHONY: clean
