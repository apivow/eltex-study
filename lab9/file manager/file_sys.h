#ifndef FILE_SYS_H
#define FILE_SYS_H

#include <limits.h>

typedef struct {
    char *name;
    int dir;
} Entry;

int is_root(const char* path);
void parent_of(const char* path, char out[PATH_MAX]);
void join_path(char out[PATH_MAX], const char* base, const char* leaf);
int load_entries(const char *path, Entry **out_list, int *out_count);
void free_entries(Entry *list, int count);

#endif