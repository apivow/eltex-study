#ifndef PANEL_H
#define PANEL_H

#include <limits.h>
#include <ncurses.h>
#include "file_sys.h"

typedef struct {
    char cwd[PATH_MAX];
    Entry *entries;
    int count; 
    int selected; 
    int scroll; 
} Panel;

void panel_init(Panel *p, const char *start_dir);
void panel_reload(Panel *p);
void panel_enter(Panel *p);
void panel_up(Panel *p);
void panel_clamp(Panel *p, int view_rows);
void panel_draw(WINDOW *win, Panel *p, int width, int height, int active);
void panel_destroy(Panel *p);

#endif