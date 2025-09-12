#define _XOPEN_SOURCE 700
#include "file_sys.h"
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int is_root (const char* p) {
    return p && p[0] == '/' && p[1] == '\0';
}

void parent_of (const char* path, char out[PATH_MAX]) {
    if (is_root(path)) {
        strcpy(out, "/");
        return;
    }
    strncpy(out, path, PATH_MAX - 1);
    out[PATH_MAX - 1] = '\0';
    char *last = strrchr(out, '/');
    if (!last) {
        strcpy(out, "/");
        return;
    }
    if (last == out) {
        out[1] = '\0';
        return;
    }
    *last = '\0';
}

void join_path(char out[PATH_MAX], const char* base, const char *leaf) {
    if (is_root(base)) snprintf(out, PATH_MAX, "/%s", leaf);
    else snprintf(out, PATH_MAX, "%s/%s", base, leaf);
}

static void free_list (Entry *list, int count) {
    if (!list) return;
    for (int i = 0; i < count; ++i) free(list[i].name);
    free(list);
}

int load_entries(const char* path, Entry **out_list, int *out_count) {
    *out_list = NULL;
    *out_count = 0;
    DIR *dir = opendir(path);
    if(!dir) return -1;
    int cnt_arr = 128, cnt = 0;
    Entry *list = (Entry *)calloc(cnt_arr, sizeof(Entry));
    if(!list) {
        closedir(dir);
        errno = ENOMEM;
        return -1;
    }

    if (!is_root(path)) {
        list[cnt].name = strdup("..");
        if (!list[cnt].name) {
            free_list(list, cnt);
            closedir(dir);
            errno = ENOMEM;
            return -1;
        }
        list[cnt].dir = 1;
        ++cnt;
    }

    struct dirent *de;
    while ((de = readdir(dir)) != NULL) {
        if(strcmp(de->d_name, ".") == 0) continue;
        if (cnt == cnt_arr) {
            int new_cnt = cnt_arr * 2;
            Entry *n1 = (Entry *)realloc(list, new_cnt* sizeof(Entry));
            if (!n1) {
                free_list(list, cnt);
                closedir(dir);
                errno = ENOMEM;
                return -1;
            }
            memset(n1 + cnt_arr, 0, (new_cnt - cnt_arr) * sizeof(Entry));
            list = n1;
            cnt_arr = new_cnt;
        }
        list[cnt].name = strdup(de->d_name);
        if (!list[cnt].name) {
            free_list(list, cnt);
            closedir(dir);
            errno = ENOMEM;
            return -1;
        }
        char full[PATH_MAX];
        join_path(full, path, de->d_name);

        struct stat st;
        list[cnt].dir = (stat(full, &st) == 0 && S_ISDIR(st.st_mode)) ? 1 : 0;
        ++cnt;
    }
    closedir(dir);
   
    *out_list = list;
    *out_count = cnt;
    return 0;
}

void free_entries(Entry *list, int n) {
    free_list(list, n);
}