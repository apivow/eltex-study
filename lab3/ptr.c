#include <stdio.h>
#include <locale.h>
#include <string.h>

void task1() {
	int number;
	printf("\nВведите положительное число: ");
	if (scanf("%d", &number) != 1 || number < 0) {
		printf("Ошибка ввода. Введите положительное число.\n");
		return 1;
	}

	unsigned char* ptr = (unsigned char*)&number;

	printf("До:\n");
	for (int i = 0; i < sizeof(int); i++) {
		printf("Байт %d: %u\n", i + 1, ptr[i]);
	}

	unsigned char new_byte;
	printf("Введите значение третьего байта (0-255): ");
	if (scanf("%hhu", &new_byte) != 1) {
		printf("Ошибка ввода\n");
		return 1;
	}

	ptr[2] = new_byte;

	printf("После:\n");
	for (int i = 0; i < sizeof(int); i++) {
		printf("Байт %d: %u\n", i + 1, ptr[i]);
	}

	printf("Новое число: %d\n", number);
}


void task2() {
	float x = 5.0;
	printf("x = %f, ", x);

	float y = 6.0;
	printf("y = %f\n", y);

	float *xp = &y; 
	float *yp = &y;
	printf("Результат: %f\n", *xp + *yp);
}


void task3() {
	int array[10];
	int num = 1;

	printf("\nВывод массива:\n");
	for (int i = 0; i < 10; i++) {
		array[i] = num++;
		printf("%3d", array[i]);
	}

	printf("\nВывод массива через указатель:\n");
	int* ptr = array;
	for (int i = 0; i < 10; i++) {
		printf("%3d", ptr[i]);
	}
}

void task4() {
	int c;
	while ((c = getchar()) != '\n' && c != EOF) {}

	char string[50];
	char substring[50];

	printf("\nВведите строку:\n");
	fgets(string, sizeof(string), stdin);
	string[strcspn(string, "\n")] = '\0';

	printf("Введите подстроку:\n");
	fgets(substring, sizeof(substring), stdin);
	substring[strcspn(substring, "\n")] = '\0';

	char* ptr = strstr(string, substring);

	if (ptr != NULL) {
		printf("Подстрока: %p\n", (void*)ptr);
	}
	else {
		printf("Указатель NULL. Подстрока не найдена.\n");
	}
}

int main() {
	setlocale(LC_ALL, "Russian");
	int choice;

	do {
		printf("\n1. Замена третьего байта.\n");
		printf("2. Изменить строку.\n");
		printf("3. Массив.\n");
		printf("4. Поиск подстроки.\n");
		printf("0. Выход\n");
		printf("\nВыберите задание:\n");
		scanf("%d", &choice);

		switch (choice) {
		case 1:
			task1();
			break;
		case 2:
			task2();
			break;
		case 3:
			task3();
			break;
		case 4:
			task4();
			break;
		case 0:
			printf("\nВыход...");
			return 0;
		default:
			printf("Неверный ввод");
		}
	} while (choice != 0);

	return 0;
}