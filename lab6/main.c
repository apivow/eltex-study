#include <stdio.h>
#include <locale.h>
#include "struct.h"

int main() {
    setlocale(LC_ALL, "ru_RU.UTF-8");

    struct Node *head = NULL;
    struct Node *tail = NULL;
    
    int count = 0;
    int choice;

    do {
        printf("1. Добавить абонента\n");
        printf("2. Удалить абонента\n");
        printf("3. Поиск абонентов по имени\n");
        printf("4. Вывод всех записей\n");
        printf("5. Выход\n");
        printf("\nВыберите пункт меню: ");
        scanf("%d", &choice);
        getchar();

        switch(choice) {
        case 1:
            struct abonent newAbonent = inputAb();
            add_abonent(&head, &tail, newAbonent, &count);
            break;
        case 2:
            int index;
            printf("\nВведите номер абонента, которого надо удалить: ");
            scanf("%d", &index);
            getchar();
            delete_abonent(&head, &tail, &count, index);
            break;
        case 3:
            search_name(head);
            break;
        case 4:
            show_list(head);
            break;
        case 5:
            printf("\nВыход...");
            return 0;
        default:
            printf("Неверный ввод\n\n");
        }
    } while (choice != 5);

    return 0;
}
