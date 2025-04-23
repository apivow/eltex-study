#include <stdio.h>
#include <locale.h>

int main () {
	setlocale(LC_ALL, "Russian");

	int number; //пользовательское число

	printf("Введите число: ");
	if (scanf("%d", &number) != 1) {
		printf("Ошибка ввода. Введите число.\n");
		return 1;
	}

	int start = 0; //маска
	int sum = 0; //количество единиц 
	int byte = number & 255; //младший байт

	printf("Двоичное представление: ");

	if (number > 0) {
		for (int i = sizeof(int) * 8 - 1; i >= 0; i--) {
			if ((number >> i) & 1) {
				start = 1;
				putchar('1');
				sum++;
			}
			else if (start) {
				putchar('0');
			}
		}
		printf("\nКоличество единиц в положительном числе: %d", sum);

		for (int i = 16; i <= 23; i++) {
			number &= ~(1 << i);
		}

		for (int i = 0; i < 8; i++) {
			if ((byte >> i) & 1)
				number |= (1 << (16 + i));
		}

		printf("\nЗаменили третий байт в положительном: %d", number);
	}
	else {
		unsigned int un = (unsigned int)number;

		for (int i = sizeof(int) * 8 - 1; i >= 0; i--) {
			if ((un >> i) & 1) {
				start = 1;
				putchar('1');
			}
			else if (start) {
				putchar('0');
			}
		}
	}
	if (!start) putchar('0');

	putchar('\n');
	return 0;
}
