#include <stdio.h>
#include <string.h>
#include "struct.h"

void input(char *str, int max_len) {
    fgets(str, max_len, stdin);

    if(strlen(str) > 8) {
        str[8] = '\0';
        printf("Строка обрезана\n");

        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }
}

void add_abonent(struct abonent *my_abonent, int *count) {
    if (*count > MAX_AB) {
        printf("Добавлено максимальное количество абонентов\n");
        return;
    }

    printf("Введите имя абонента: ");
    input(my_abonent[*count].name, 10);
    printf("Введите фамилию абонента: ");
    input(my_abonent[*count].second_name, 10);
    printf("Введите номер телефона абонента: ");
    input(my_abonent[*count].tel, 10);
    printf("\n");

    (*count)++;
}

void delete_abonent(struct abonent *my_abonent, int *count, int index) {
    if (index < 0 || index >= *count) {
        printf("Некорректный индекс абонента\n\n");
        return;
    }

    my_abonent[index].name[0] = '\0';
    my_abonent[index].second_name[0] = '\0';
    my_abonent[index].tel[0] = '\0';

    for (int i = index; i < *count - 1; i++) {
        my_abonent[i] = my_abonent[i + 1];
    }

    (*count)--;
}

void show_list(struct abonent *my_abonent, int count) {
    if (count == 0) {
        printf("Список пуст\n\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        printf("Абонент №%d\n"
        "Имя: %s Фамилия: %s Телефон: %s", i + 1, my_abonent[i].name, my_abonent[i].second_name, my_abonent[i].tel);
        printf("\n\n");
    }
}

void search_name(struct abonent *my_abonent, int count) {
    if (count ==  0) {
        printf("Список пуст\n\n");
        return;
    }

    char name[10];

    printf("Введите имя для поиска: ");
    input(name, 10);

    int found = 0;
    for(int i = 0; i < count; i++) {
        if(strcmp(name, my_abonent[i].name) == 0) {
            printf("Абонент №%d\n"
            "Имя: %s Фамилия: %s Телефон: %s", i + 1, my_abonent[i].name, my_abonent[i].second_name, my_abonent[i].tel);
            printf("\n\n");
            found = 1;
        }
    }

    if (!found) {
        printf("Абонент с таким именем не найден\n\n");
        return;
    }
}
