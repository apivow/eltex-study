#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <ncurses.h>

#define SHM_NAME "/chat_shm"
#define NAME_LEN 32
#define TEXT_LEN 256
#define MAX_CLIENT 16
#define MAX_MSG 512
#define MAX_INBOX 128
#define MSG_SYS 0
#define MSG_CHAT 1
#define INBOX_JOIN 0
#define INBOX_CHAT 1

typedef struct {
    int id;
    int type;
    char name[NAME_LEN];
    char text[TEXT_LEN];
} Msg;

typedef struct {
    int type;
    char name[NAME_LEN];
    char text[TEXT_LEN];
} InboxItem;

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int init;
    int client_cnt;
    char client_name[MAX_CLIENT][NAME_LEN];
    int next_msg_id;
    int msg_cnt;
    Msg msg[MAX_MSG];
    int inbox_head;
    int inbox_tail;
    InboxItem inbox[MAX_INBOX];
} ChatShm;

static ChatShm *g = NULL;
static int shm_fd = -1;

static WINDOW *chatwin = NULL;
static WINDOW *inputwin = NULL;
static pthread_mutex_t ui_mutex = PTHREAD_MUTEX_INITIALIZER;

static char g_name[NAME_LEN] = {0};
static volatile int g_run = 1;
static int g_last_seen_id = 0;

static int inbox_full() {
    int next_tail = (g->inbox_tail + 1) % MAX_INBOX;
    return next_tail == g->inbox_head;
}

static void inbox_push(int type, const char *name, const char *text) {
    while (inbox_full()) pthread_cond_wait(&g->cond, &g->mutex);

    g->inbox[g->inbox_tail].type = type;
    strncpy(g->inbox[g->inbox_tail].name, name ? name : "", NAME_LEN - 1);
    g->inbox[g->inbox_tail].name[NAME_LEN - 1] = '\0';
    if(text) {
        strncpy(g->inbox[g->inbox_tail].text, text, TEXT_LEN - 1);
        g->inbox[g->inbox_tail].text[TEXT_LEN - 1] = '\0';
    } else {
        g->inbox[g->inbox_tail].text[0] = '\0';
    }
    g->inbox_tail = (g->inbox_tail + 1) % MAX_INBOX;
    pthread_cond_broadcast(&g->cond);
}

static void ui_print_line(const char *line) {
    pthread_mutex_lock(&ui_mutex);
    if (chatwin) {
        wprintw(chatwin, "%s\n", line);
        wrefresh(chatwin);
    }
    pthread_mutex_unlock(&ui_mutex);
}

static void ui_print_msg(const Msg *m) {
    char buf[NAME_LEN + TEXT_LEN + 16];
    if (m->type == MSG_SYS) {
        snprintf(buf, sizeof(buf), "%s", m->text);
    } else {
        snprintf(buf, sizeof(buf), "%s: %s", m->name, m->text);
    }
    //ui_print_line(buf);
    pthread_mutex_lock(&ui_mutex);
    wprintw(chatwin, "%s\n", buf);
    wrefresh(chatwin);
    pthread_mutex_unlock(&ui_mutex);
}

static void* receiver_thread(void *arg) {
    (void)arg;
    pthread_mutex_lock(&g->mutex);
    int start_id = g->next_msg_id - g->msg_cnt + 1;
    if (start_id < 1) start_id = 1;
    for (int id = 1; id <= g->next_msg_id; id++) {
        int idx = (id - 1) % MAX_MSG;
        ui_print_msg(&g->msg[idx]);
    }
    g_last_seen_id = g->next_msg_id;
    pthread_mutex_unlock(&g->mutex);


    while (g_run) {
        pthread_mutex_lock(&g->mutex);
        while (g_run && g_last_seen_id >= g->next_msg_id) {
            pthread_cond_wait(&g->cond, &g->mutex);
        }
        while (g_last_seen_id < g->next_msg_id) {
            g_last_seen_id++;
            int idx = (g_last_seen_id - 1) % MAX_MSG;
            ui_print_msg(&g->msg[idx]);
        }

        pthread_mutex_unlock(&g->mutex);

        pthread_mutex_lock(&ui_mutex);
        if (inputwin) {
            box(inputwin, 0, 0);
            mvwprintw(inputwin, 1, 2, "%s> ", g_name);
            wrefresh(inputwin);
        }
        pthread_mutex_unlock(&ui_mutex);
    }
    return NULL;
}

static void init_ui() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    int h = LINES - 3;
    int w = COLS;
    chatwin = newwin(h, w, 0, 0);
    scrollok(chatwin, TRUE);
    box(chatwin, 0, 0);
    wrefresh(chatwin);

    inputwin = newwin(3, w, h, 0);
    box(inputwin, 0, 0);
    mvwprintw(inputwin, 1, 2, "Name: ");
    wmove(inputwin, 1, 8);
    wrefresh(inputwin);
}

static void teardown_ui() {
    if (inputwin) delwin(inputwin);
    if (chatwin) delwin(chatwin);
    endwin();
}

int main() {
    init_ui();
    pthread_mutex_lock(&ui_mutex);
    echo();
    char namebuf[NAME_LEN];
    memset(namebuf, 0, sizeof(namebuf));
    wgetnstr(inputwin, namebuf, NAME_LEN - 1);
    noecho();
    pthread_mutex_unlock(&ui_mutex);
    if (namebuf[0] == '\0') {
        strncpy(namebuf, "anon", NAME_LEN - 1);
        namebuf[NAME_LEN - 1] = '\0';
    }
    strncpy(g_name, namebuf, NAME_LEN - 1);
    g_name[NAME_LEN - 1] = '\0';

    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        teardown_ui();
        perror("shm_open");
        return 1;
    }
    g = (ChatShm*)mmap(NULL, sizeof(ChatShm), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (g == MAP_FAILED) {
        teardown_ui();
        perror("mmap");
        return 1;
    }

    while (!g->init) { usleep(1000); }

    pthread_mutex_lock(&g->mutex);
    inbox_push(INBOX_JOIN, g_name, NULL);
    pthread_mutex_unlock(&g->mutex);

    pthread_t thr;
    if (pthread_create(&thr, NULL, receiver_thread, NULL) != 0) {
        teardown_ui();
        perror("pthread_creat");
        return 1;
    }

    while (g_run) {
        pthread_mutex_lock(&ui_mutex);
        werase(inputwin);
        box(inputwin, 0, 0);
        mvwprintw(inputwin, 1, 2, "%s> ", g_name);
        wmove(inputwin, 1, 3 + (int)strlen(g_name));
        wrefresh(inputwin);

        echo();
        char line[TEXT_LEN];
        memset(line, 0, sizeof(line));
        wgetnstr(inputwin, line, TEXT_LEN - 1);
        noecho();
        pthread_mutex_unlock(&ui_mutex);

        if (strcmp(line, "/quit") == 0 || strcmp(line, "/exit") == 0) {
            g_run = 0;
            pthread_mutex_lock(&g->mutex);
            pthread_cond_broadcast(&g->cond);
            pthread_mutex_unlock(&g->mutex);
            break;
        }
        if (line[0] == '\0') continue;

        pthread_mutex_lock(&g->mutex);
        inbox_push(INBOX_CHAT, g_name, line);
        pthread_mutex_unlock(&g->mutex);
    }
    pthread_join(thr, NULL);
    teardown_ui();
    if (g) munmap(g, sizeof(ChatShm));
    if (shm_fd != - 1) close (shm_fd);
    return 0;
}