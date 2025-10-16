#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define SERVER_QUEUE "/chat_server"
#define MAX_MSG 1024
#define MAX_NAME 32
#define MAX_LINE 500
#define MAX_QNAME 64

static mqd_t server_q = (mqd_t) - 1;
static mqd_t client_q = (mqd_t) - 1;
static char client_name[MAX_NAME];
static char client_qname[MAX_QNAME];

static WINDOW *msg_win, *input_win;
static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

static char line[MAX_LINE][MAX_MSG];
static int line_cnt = 0;

static void add_line(const char* s) {
    pthread_mutex_lock(&m);
    if (line_cnt < MAX_LINE) {
        strncpy(line[line_cnt], s, MAX_MSG - 1);
        line[line_cnt][MAX_MSG - 1] = '\0';
        line_cnt++;
    } else {
        memmove(line, line + 1, sizeof(line[0]) * (MAX_LINE - 1));
        strncpy(line[MAX_LINE - 1], s, MAX_MSG - 1);
        line[MAX_LINE - 1][MAX_MSG - 1] = '\0';
    }
    pthread_mutex_unlock(&m);
}

static void redraw() {
    pthread_mutex_lock(&m);
    werase(msg_win);
    int h, w;
    getmaxyx(msg_win, h, w);
    int start = (line_cnt > h - 1) ? (line_cnt - (h - 1)) : 0;
    for (int i = start; i < line_cnt; i++) mvwprintw(msg_win, i - start, 0, "%s", line[i]);
    wrefresh(msg_win);
    pthread_mutex_unlock(&m);
}

static void on_sigint(int sig) {
    (void)sig;
    endwin();
    if (server_q != (mqd_t) - 1) mq_close(server_q);
    if (client_q != (mqd_t) - 1) {
        mq_close(client_q);
        mq_unlink(client_qname);
    } 
    exit(0);
}

static void* recv_thread(void* arg) {
    (void)arg;
    char buf[MAX_MSG];
    while(1) {
        ssize_t n = mq_receive(client_q, buf, sizeof(buf), NULL);
        if (n < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                usleep(50000);
                continue;
            }
            continue;
        }
        add_line(buf);
        redraw();
    }
    return NULL;
}

static void* send_thread(void* arg) {
    (void)arg;
    char in[MAX_MSG];
    char out[MAX_MSG];
    while(1) {
        werase(input_win);
        mvwprintw(input_win, 0, 0, ">");
        wrefresh(input_win);

        wgetnstr(input_win, in, (int)sizeof(in) - 1);
        if (in[0] == '\0') continue;
        if (strcmp(in, "/quit") == 0) on_sigint(0);

        int n = snprintf(out, sizeof(out), "MSG:%.*s:%.*s", MAX_NAME - 1, client_name, (int)sizeof(in) - 1, in);
        if (n > 0 && n < (int)sizeof(out)) mq_send(server_q, out, n + 1, 0);
    }
    return NULL;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <name>\n", argv[0]);
        return 1;
    }

    strncpy(client_name, argv[1], MAX_NAME - 1);
    client_name[MAX_NAME - 1] = '\0';

    signal(SIGINT, on_sigint);

    snprintf(client_qname, sizeof(client_qname), "/cli_%d", getpid());
    mq_unlink(client_qname);

    struct mq_attr attr = {0};
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG;

    client_q = mq_open(client_qname, O_CREAT | O_RDWR | O_EXCL, 0666, &attr);
    if (client_q == (mqd_t) - 1) {
        perror("mq_open client");
        return 1;
    }

    server_q = mq_open(SERVER_QUEUE, O_WRONLY);
    if (server_q == (mqd_t) - 1) {
        perror("mq_open server");
        mq_close(client_q);
        mq_unlink(client_qname);
        return 1;
    }

    char join[MAX_MSG];
    int j = snprintf(join, sizeof(join), "JOIN:%.*s:%.*s", MAX_NAME - 1, client_name, (int)sizeof(client_qname) - 1, client_qname);
    if (j > 0 && j < (int)sizeof(join)) mq_send(server_q, join, j + 1, 0);

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    int H = LINES, W = COLS;
    msg_win = newwin(H - 3, W, 0, 0);
    input_win = newwin(3, W, H - 3, 0);
    scrollok(msg_win, TRUE);
    box(input_win, 0, 0);
    wrefresh(input_win);

    pthread_t th_r, th_s;
    pthread_create(&th_r, NULL, recv_thread, NULL);
    pthread_create(&th_s, NULL, send_thread, NULL);

    while(1) {
        box(input_win, 0, 0);
        wrefresh(input_win);
        usleep(100000);
    }
    return 0;
}