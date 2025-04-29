#include "stdio.h"
#include "locale.h"
#define N 5

void array_output() {
	int array[N][N];
	int num = 1;

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			array[i][j] = num;
			num++;
		}
	}

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			printf("%3d", array[i][j]);
		}
		printf("\n");
	}
}

void array_reverse() {
	int array[N];

	printf("Введите %d чисел: \n", N);
	for (int i = 0; i < N; i++) {
		scanf("%d", &array[i]);
	}

	for (int i = 0; i < N / 2; i++) {
		int temp = array[i];
		array[i] = array[N - 1 - i];
		array[N - 1 - i] = temp;
	}

	for (int i = 0; i < N; i++) {
		printf("%2d", array[i]);
	}
	printf("\n");
}

void triangle() {
	int array[N][N];

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			if (i + j < N - 1)
				array[i][j] = 0;
			else
				array[i][j] = 1;
		}
	}

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			printf("%3d", array[i][j]);
		}
		printf("\n");
	}
}

void snail() {
	int array[N][N];
	int num = 1; // число от 1 до n^2
	int up = 0, down = N - 1, left = 0, right = N - 1; //границы

	while (num <= N * N) {
		//вправо
		for (int j = left; j <= right && num <= N * N; j++) {
			array[up][j] = num++;
		}
		up++;

		//вниз 
		for (int i = up; i <= down && num <= N * N; i++) {
			array[i][right] = num++;
		}
		right--;

		//влево 
		for (int j = right; j >= left && num <= N * N; j--) {
			array[down][j] = num++;
		}
		down--;

		//вверх
		for (int i = down; i >= up && num <= N * N; i--) {
			array[i][left] = num++;
		}
		left++;
	}

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			printf("%3d", array[i][j]);
		}
		printf("\n");
	}
}

int main() {
	setlocale(LC_ALL, "Russian");
	int choice;

	do {
		printf("1. Вывести квадратную матрицу по заданному N.\n");
		printf("2. Вывести заданный массив размером N в обратном порядке.\n");
		printf("3. Заполнить верхний треугольник матрицы 1, а нижний 0.\n");
		printf("4. Заполнить матрицу числами от 1 до N^2 улиткой.\n");
		printf("5. Выход.\n");
		printf("Выберите задание: \n");
		scanf("%d", &choice);

		switch (choice) {
		case 1:
			array_output();
			break;
		case 2:
			array_reverse();
			break;
		case 3:
			triangle();
			break;
		case 4:
			snail();
			break;
		case 5:
			printf("\nВыход");
			return 0;
		default:
			printf("Неверный ввод.");
		}
	} while (choice != 5);

	return 0;
}