#include <stdio.h>

int num() {
    int num;
    while(1) {
        printf("Введите число: ");
        if (scanf("%d", &num) == 1) {
            break;
        } else {
            printf("Ошибка. Введите корретное число.\n");
            while(getchar() != '\n');
        }
    }
    return num;
}