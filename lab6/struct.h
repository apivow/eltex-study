#ifndef STRUCT_H
#define STRUCT_H

struct abonent {
    char name[10];
    char second_name[10];
    char tel[10];
};

struct Node {
    struct abonent data;
    struct Node *prev;
    struct Node *next;
};

struct abonent inputAb();
struct Node* createNode(struct abonent my_abonent);
void input(char *str, int max_len);
void add_abonent(struct Node **head, struct Node **tail, struct abonent my_abonent, int *count);
void delete_abonent(struct Node **head, struct Node **tail, int *count, int index);
void search_name(struct Node *head);
void show_list(struct Node *head);

#endif