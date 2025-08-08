#include <stdio.h>
#include <stdlib.h>
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

struct abonent inputAb() {
    struct abonent my_abonent;
    printf("Введите имя: ");
    input(my_abonent.name, 10);
    my_abonent.name[strcspn(my_abonent.name, "\n")] = 0;
    printf("Введите фамиилию: ");
    input(my_abonent.second_name, 10);
    my_abonent.second_name[strcspn(my_abonent.second_name, "\n")] = 0;
    printf("Введите номера телефона: ");
    input(my_abonent.tel, 10);
    my_abonent.tel[strcspn(my_abonent.tel, "\n")] = 0;

    return my_abonent;
}

struct Node* createNode(struct abonent my_abonent) {
    struct Node *newNode = (struct Node*)malloc(sizeof(struct Node));
    if (newNode != NULL) {
        newNode->data = my_abonent;
        newNode->prev = NULL;
        newNode->next = NULL;
    }
    return newNode;
};

void add_abonent(struct Node **head, struct Node **tail, struct abonent my_abonent, int *count) {
    struct Node *newNode = createNode(my_abonent);
    if (newNode == NULL) return;
    if (*head == NULL) {
        *head = newNode;
        *tail = newNode;
    } else {
        newNode->prev = *tail;
        (*tail)->next = newNode;
        *tail = newNode;
    }
    (*count)++;
}

void delete_abonent(struct Node **head, struct Node **tail, int *count, int index) {
    if (*head == NULL) return;
    if (index < 0 || index >= *count) return;
    struct Node *curr = *head;
    int i = 0;

    while (i < index) {
        curr = curr->next;
        i++;
    }

    if (curr == *head) {
        *head = curr->next;
        if (*head != NULL) {
            (*head)->prev = NULL;
        } else {
            *tail = NULL;
        }
    } else if (curr == *tail) {
        *tail = curr->prev;
        (*tail)->next = NULL;
    } else {
        curr->prev->next = curr->next;
        curr->next->prev = curr->prev;
    }

    free(curr);
    (*count)--;
}

void show_list(struct Node *head) {
   if (head == NULL) return;

   struct Node *curr = head;
   int index = 0;

   while (curr != NULL) {
    printf("%d Имя: %s Фамилия: %s Номер телефона: %s\n", index, curr->data.name, curr->data.second_name, curr->data.tel);
    curr = curr->next;
    index++;
   }
}

void search_name(struct Node *head) {
    if (head == NULL) return;

    char searchAb[10];
    printf("Введите имя для поиска: ");
    input(searchAb, 10);
    searchAb[strcspn(searchAb, "\n")] = 0;

    struct Node *curr = head;
    int index = 0;
    int f = 0;

    while (curr != NULL) {
        if (strcasecmp(curr->data.name, searchAb) == 0) {
            printf("%d Имя: %s Фамилия: %s Номер телефона: %s\n", index, curr->data.name, curr->data.second_name, curr->data.tel);
            f = 1;
        }
        curr = curr->next;
        index++;
    }
    if (f == 0) printf ("Абоненты не найдены.\n");  
}