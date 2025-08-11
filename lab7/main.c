#include <stdio.h>
#include "calculator.h"
#include <locale.h>

int main() {
    setlocale(LC_ALL, "Russia");
    int a, b, res;

    int choice;

    do {
        printf("\n\nВыберите пункт меню: \n\n");
        printf("1. Сложение\n");
        printf("2. Вычитание\n");
        printf("3. Умножение\n");
        printf("4. Деление\n");
        printf("5. Выход\n");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            a = num();
            b = num();
            res = add(a, b);
            printf("\n%d + %d = %d", a, b, res);
            break;
        case 2:
            a = num();
            b = num();
            res = sub(a, b);
            printf("\n%d - %d = %d", a, b, res);
            break;
        case 3:
            a = num();
            b = num();
            res = mult(a, b);
            printf("\n%d * %d = %d", a, b, res);
            break;
        case 4:
            a = num();
            b = num();
            if (b != 0) {
                res = div(a, b);
                printf("\n%d : %d = %d", a, b, res);
            } else {
                printf("Ошибка. На ноль делить нельзя.\n");
                break;
            }
            break;
        case 5:
            printf("Выход...\n");
            return 0;
        default:
            printf("Неверный ввод.\n");
        }
    } while (choice != 5);
    return 0;
}
