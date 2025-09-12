#define _XOPEN_SOURCE 700
#include <ncurses.h>
#include "panel.h"

int main() {
    Panel left, right;
    panel_init(&left, NULL);
    panel_init(&right, NULL);

    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);

    int active_left = 1;

    while(1) {
        int rows, cols;
        getmaxyx(stdscr, rows, cols);

        int left_w = cols / 2;
        int right_w = cols - left_w;

        WINDOW *lw = newwin(rows, left_w, 0, 0);
        WINDOW *rw = newwin(rows, right_w, 0, left_w);

        panel_draw(lw, &left, left_w, rows, active_left);
        panel_draw(rw, &right, right_w, rows, !active_left);

        int ch = wgetch(stdscr);
        if (ch == 'q' || ch == 'Q') break;
        Panel *p = active_left ? &left : &right;

        if (ch == '\t') active_left = !active_left;
        else if (ch == KEY_UP) {
            p->selected--;
            panel_clamp(p, rows - 2);
        }
        else if (ch == KEY_DOWN) {
            p->selected++;
            panel_clamp(p, rows - 2);
        }
        else if (ch == KEY_PPAGE) {
            p->selected -= (rows - 2);
            panel_clamp(p, rows - 2);
        }
        else if (ch == KEY_NPAGE) {
            p->selected += (rows - 2);
            panel_clamp(p, rows - 2);
        }
        else if (ch == KEY_HOME) {
            p->selected = 0;
            panel_clamp(p, rows - 2);
        }
        else if (ch == KEY_END) {
            p->selected = p->count - 1;
            panel_clamp(p, rows - 2);
        }
        else if (ch == 10 || ch == KEY_ENTER) panel_enter(p);
        else if (ch = KEY_BACKSPACE || ch == 127 || ch == 8) panel_up(p);

    }
    endwin();
    panel_destroy(&left);
    panel_destroy(&right);
    return 0;
}