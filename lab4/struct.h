#ifndef STRUCT_H
#define STRUCT_H
#define MAX_AB 100

struct abonent {
    char name[10];
    char second_name[10];
    char tel[10];
};

void input(char *str, int max_len);
void add_abonent(struct abonent *my_abonent, int *count);
void delete_abonent(struct abonent *my_abonent, int *count, int index);
void search_name();
void show_list(struct abonent *my_abonent, int count);

#endif
