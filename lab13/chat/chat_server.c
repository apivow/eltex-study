#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define SERVER_QUEUE "/chat_server"
#define MAX_CLIENT 32
#define MAX_MSG 1024
#define MAX_HISTORY 200

typedef struct {
    char name[32];
    char qname[64];
    mqd_t q;
    int used;
} Client;

static Client client[MAX_CLIENT];
static int num_client = 0;
static char history[MAX_HISTORY][MAX_MSG];
static int history_cnt = 0;
static mqd_t server_q = (mqd_t) - 1;

static void add_history(const char* text) {
    if (history_cnt < MAX_HISTORY) {
        strncpy(history[history_cnt], text, MAX_MSG - 1);
        history[history_cnt][MAX_MSG - 1] = '\0';
        history_cnt++;
    }
}

static void broadcast(const char* text, const char* exclude_name) {
    for (int i = 0; i < num_client; i++) {
        if (!client[i].used) continue;
        if (exclude_name && strcmp(client[i].qname, exclude_name) == 0) continue;
        mq_send(client[i].q, text, strnlen(text, MAX_MSG - 1) + 1, 0);
    }
}

static int add_client(const char* name, const char* qname) {
    if (num_client >= MAX_CLIENT) return -1;
    mqd_t q = mq_open(qname, O_WRONLY);
    if (q == (mqd_t) - 1) return -1;

    Client c = {0};
    strncpy(c.name, name, sizeof(c.name) - 1);
    strncpy(c.qname, qname, sizeof(c.qname) - 1);
    c.q = q;
    c.used = 1;
    client[num_client++] = c;

    for (int i = 0; i < history_cnt; i++) mq_send(q, history[i], strnlen(history[i], MAX_MSG - 1) + 1, 0);

    char msg[MAX_MSG];
    snprintf(msg, sizeof(msg), "SERVER: %s joined", name);
    add_history(msg);
    broadcast(msg, NULL);
    printf("%s\n", msg);
    return 0;
}

static void on_sigint(int sig) {
    (void)sig;
    printf("\nServer close\n");
    for (int i = 0; i < num_client; i++) {
        if (client[i].used) mq_close(client[i].q);
    }
    if (server_q != (mqd_t) - 1) {
        mq_close(server_q);
        mq_unlink(SERVER_QUEUE);
    }
    exit(0);
}

int main() {
    signal(SIGINT, on_sigint);
    mq_unlink(SERVER_QUEUE);
    struct mq_attr attr = {0};
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG;

    server_q = mq_open(SERVER_QUEUE, O_CREAT | O_RDONLY | O_EXCL, 0666, &attr);
    if (server_q == (mqd_t) - 1) {
        perror("mq_open server");
        return 1;
    }

    printf("Server start. Queue: %s\n", SERVER_QUEUE);
    
    char buf[MAX_MSG];
    while(1) {
        ssize_t n = mq_receive(server_q, buf, sizeof(buf), NULL);
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("mq_receive");
            continue;
        }
        if (strncmp(buf, "JOIN:", 5) == 0) {
            char name[32] = {0};
            char qname[64] = {0};
            const char *p = buf + 5;
            const char* colon = strchr(p, ':');
            if (colon) {
                size_t ln = (size_t)(colon - p);
                if (ln >= sizeof(name)) ln = sizeof(name) - 1;
                memcpy(name, p, ln);
                strncpy(qname, colon + 1, sizeof(qname) - 1);
                add_client(name, qname);
            }
            continue;
        }

        if (strncmp(buf, "MSG:", 4) == 0) {
            const char* p = buf + 4;
            const char* colon = strchr(p, ':');
            if (colon) {
                char name[32] = {0};
                size_t ln = (size_t)(colon - p);
                if (ln >= sizeof(name)) ln = sizeof(name) - 1;
                memcpy(name, p, ln);

                char out[MAX_MSG];
                snprintf(out, sizeof(out), "%.*s: %s", (int)ln, p, colon + 1);
                add_history(out);
                broadcast(out, NULL);
                printf("Broadcast: %s\n", out);
            }
            continue;
        }
    }
    return 0;
}