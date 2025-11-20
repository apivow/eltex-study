#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

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

static void clean(){
    if(g) {
        munmap(g, sizeof(ChatShm));
        g = NULL;
    }
    if (shm_fd != -1) {
        close(shm_fd);
        shm_fd = -1;
    }
    shm_unlink(SHM_NAME);
}

static void on_sigint(int sig) {
    (void)sig;
    clean();
    _exit(0);
}

static void append_msg(int type, const char *name, const char *text) {
    int id = ++g->next_msg_id;
    int idx = (id - 1) % MAX_MSG;
    g->msg[idx].id = id;
    g->msg[idx].type = type;
    strncpy(g->msg[idx].name, name ? name : "server", NAME_LEN - 1);
    g->msg[idx].name[NAME_LEN - 1] = '\0';
    strncpy(g->msg[idx].text, text ? text: "", TEXT_LEN - 1);
    g->msg[idx].text[TEXT_LEN - 1] = '\0';
    if (g->msg_cnt < MAX_MSG) g->msg_cnt++;
}

static int inbox_empty() {
    return g->inbox_head == g->inbox_tail;
}

static int inbox_pop(InboxItem *out) {
    if (inbox_empty()) return 0;
    *out = g->inbox[g->inbox_head];
    g->inbox_head = (g->inbox_head + 1) % MAX_INBOX;
    return 1;
}

static int find_client(const char *name) {
    for (int  i = 0; i < g->client_cnt; i++) {
        if (strncmp(g->client_name[i], name, NAME_LEN) == 0) return i;
    }
    return -1;
}

int main() {
    signal(SIGINT, on_sigint);
    signal(SIGTERM, on_sigint);

    shm_unlink(SHM_NAME);

    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }
    if (ftruncate(shm_fd, sizeof(ChatShm)) == -1) {
        perror("ftruncate");
        clean();
        return 1;
    }

    g = (ChatShm*)mmap(NULL, sizeof(ChatShm), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (g == MAP_FAILED) {
        perror("mmap");
        clean();
        return 1;
    }

    memset(g, 0, sizeof(ChatShm));

    pthread_mutexattr_t ma;
    pthread_condattr_t ca;
    pthread_mutexattr_init(&ma);
    pthread_mutexattr_setpshared(&ma, PTHREAD_PROCESS_SHARED);
    pthread_condattr_init(&ca);
    pthread_condattr_setpshared(&ca, PTHREAD_PROCESS_SHARED);

    if (pthread_mutex_init(&g->mutex, &ma) != 0) {
        perror("pthread_mutex_init");
        clean();
        return 1;
    }
    if (pthread_cond_init(&g->cond, &ca) != 0) {
        perror("pthread_cond_init");
        clean();
        return 1;
    }

    pthread_mutexattr_destroy(&ma);
    pthread_condattr_destroy(&ca);
    g->init = 1;
    printf("Server ready. Shared memory: %s\n", SHM_NAME);

    pthread_mutex_lock(&g->mutex);
    append_msg(MSG_SYS, "sever", "Server start");
    pthread_cond_broadcast(&g->cond);
    pthread_mutex_unlock(&g->mutex);

    while(1) {
        pthread_mutex_lock(&g->mutex);
        while(inbox_empty()) {
            pthread_cond_wait(&g->cond, &g->mutex);
        }

        InboxItem item;
        while (inbox_pop(&item)) {
            if (item.type == INBOX_JOIN) {
                if (find_client(item.name) == -1) {
                    if (g->client_cnt < MAX_CLIENT) {
                        strncpy(g->client_name[g->client_cnt], item.name, NAME_LEN - 1);
                        g->client_name[g->client_cnt][NAME_LEN - 1] = '\0';
                        g->client_cnt++;
                        char buf[TEXT_LEN];
                        snprintf(buf, sizeof(buf), "[join] %s logged into the chat", item.name);
                        append_msg(MSG_SYS, "server", buf);
                        printf("Server: %s\n", buf);
                        fflush(stdout);
                        pthread_cond_broadcast(&g->cond);
                    } else {
                        char buf[TEXT_LEN];
                        snprintf(buf, sizeof(buf), "[warn] no place for the client: %s", item.name);
                        append_msg(MSG_SYS, "server", buf);
                        printf("Server: %s\n", buf);
                        fflush(stdout);
                        pthread_cond_broadcast(&g->cond);
                    }
                }
            } else if (item.type == INBOX_CHAT) {
                append_msg(MSG_CHAT, item.name, item.text);
                printf("%s: %s\n", item.name, item.text);
                fflush(stdout);
                pthread_cond_broadcast(&g->cond);
            }
        }
        pthread_cond_broadcast(&g->cond);
        pthread_mutex_unlock(&g->mutex);
    }
    clean();
    return 0;
}