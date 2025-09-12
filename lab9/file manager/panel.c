#define _XOPEN_SOURCE 700
#include "panel.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static void draw_title(WINDOW *w, const char *path, int width, int active) {
    wattron(w, A_BOLD | (active ? A_REVERSE : 0));
    mvwprintw(w, 0, 1, " %s ", path);
    wattroff(w, A_BOLD | (active ? A_REVERSE : 0));
    for (int x = (int)strlen(path) + 3; x < width - 2; ++x) mvwaddch(w, 0, x, ' ');
}

void panel_init(Panel *p, const char *start_dir) {
    memset(p, 0, sizeof(*p));
    if (start_dir && *start_dir) strncpy(p->cwd, start_dir, sizeof(p->cwd) - 1);
    else getcwd(p->cwd, sizeof(p->cwd));
    panel_reload(p);
}

void panel_reload(Panel *p) {
    Entry *list = NULL;
    int n = 0;
    if (load_entries(p->cwd, &list, &n) != 0) {
        free_entries(p->entries, p->count);
        p->entries = NULL;
        p->count = 0;
        p->selected = p->scroll = 0;
        return;
    }
    free_entries(p->entries, p->count);
    p->entries = list;
    p->count = n;
    if (p->selected >= p->count) p->selected = p->count -1;
    if (p->selected < 0) p->selected = 0;
    if (p->scroll > p->selected) p->scroll = p->selected;
    if (p->scroll < 0) p->scroll = 0;
}

void panel_clamp(Panel *p, int view_rows) {
    if (p->count == 0) {
        p->selected = 0;
        p->scroll = 0;
        return;
    }
    if (p->selected < 0) p->selected = 0;
    if (p->selected >= p->count) p->selected = p->count - 1;
    if (p->selected < p->scroll) p->scroll = p->selected;
    if (p->selected >= p->scroll + view_rows) p->scroll = p->selected - view_rows + 1;
    if (p->scroll < 0) p->scroll = 0;
    if (p->scroll > p->count - view_rows) {
        p->scroll = p->count - view_rows;
        if (p->scroll < 0) p->scroll = 0;
    }
}

void panel_enter(Panel *p) {
    if (p->count == 0) return;
    Entry *e = &p->entries[p->selected];
    if (!e->dir) return;
    if(strcmp(e->name, "..") == 0) {
        char parent[PATH_MAX];
        parent_of(p->cwd, parent);
        strcpy(p->cwd, parent);
    } else {
        char next[PATH_MAX];
        join_path(next, p->cwd, e->name);
        strcpy(p->cwd, next);
    }
        p->selected = 0;
        p->scroll = 0;
        panel_reload(p);
}

void panel_up(Panel *p) {
    if (is_root(p->cwd)) return;
    char parent[PATH_MAX];
    parent_of(p->cwd, parent);
    strcpy(p->cwd, parent);
    p->selected = 0;
    p->scroll = 0;
    panel_reload(p);
}

void panel_draw(WINDOW *win, Panel *p, int width, int heigth, int active) {
    werase(win);
    box(win, 0, 0);
    draw_title(win, p->cwd, width, active);
    int list_rows = heigth - 2;
    if (list_rows < 1) list_rows = 1;
    panel_clamp(p, list_rows);

    for (int row = 0; row < list_rows; ++row) {
        int idx = p->scroll + row;
        if (idx >= p->count) break;
        int y = 1 + row;
        int selected = (idx == p->selected);
        mvwaddch(win, y, 1, selected ? '>' : ' ');
        char namebuf[PATH_MAX + 2];
        if (p->entries[idx].dir) snprintf(namebuf, sizeof(namebuf), "%s/", p->entries[idx].name);
        else snprintf(namebuf, sizeof(namebuf), "%s", p->entries[idx].name);
        mvwprintw(win, y, 3, "%.*s", width - 5, namebuf);
    }
    wattron(win, A_DIM);
    mvwprintw(win, heigth - 1, 2, "Enter: open. Backspace: up. Q/q: quit");
    wattroff(win, A_DIM);
    wrefresh(win);
}

void panel_destroy(Panel *p) {
    free_entries(p->entries, p->count);
    p->entries = NULL;
    p->count = 0;
}