#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define PORT 7777
#define START 2
#define MAX 16
#define BACKLOG 128
#define BUF 1024
#define IDLE_TIMEOUT 10

struct TaskMsg {
    long mtype;
    int num;
    char ip[64];
    int port;
};

static int msqid = -1;

static int create_listen_socket(uint16_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        exit(1);
    }
    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (bind(fd, (struct sockaddr*)&addr, (socklen_t)sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }
    if (listen(fd, BACKLOG) < 0) {
        perror("listen");
        exit(1);
    }
    return fd;
}

static ssize_t read_line(int fd, char *buf, size_t cap) {
    size_t i = 0;
    while (i + 1 < cap) {
        char c;
        ssize_t n = read(fd, &c, 1);
        if (n == 0) break;
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        buf[i++] = c;
        if (c == '\n') break;
    }
    buf[i] = '\0';
    return (ssize_t)i;
}

static ssize_t write_all(int fd, const void *buf, size_t len) {
    const char *p = (const char*) buf;
    size_t left = len;
    while (left > 0) {
        ssize_t n = write(fd, p, left);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        p += n;
        left -= (size_t)n;
    }
    return (ssize_t)len;
}

static long long sum_1_to_n(long long n) {
    if (n < 0) return 0;
    return (n * (n + 1) / 2);
}

static void worker_loop() {
    while (1) {
        struct TaskMsg t;
        ssize_t r = msgrcv(msqid, &t, sizeof(t) - sizeof(long), 1, 0);
        if (r < 0) {
            if (errno == EINTR) continue;
            perror("msgrcv");
            exit(1);
        }
        fprintf(stderr, "Worker %d got N %d -> %s:%d\n", (int)getpid(), t.num, t.ip, t.port); //
        long long res = sum_1_to_n(t.num);

        int s = socket(AF_INET, SOCK_STREAM, 0);
        if (s < 0) {
            perror("soclet");
            continue;
        }
        struct sockaddr_in cli;
        memset(&cli, 0, sizeof(cli));
        cli.sin_family = AF_INET;
        cli.sin_port = htons((uint16_t)t.port);
        if (inet_pton(AF_INET, t.ip, &cli.sin_addr) != 1) {
            close(s);
            continue;
        }
        if(connect(s, (struct sockaddr*)&cli, sizeof(cli)) == 0) {
            char out[64];
            int m = snprintf(out, sizeof(out), "Res %lld\n", res);
            if (m > 0) write_all(s, out, (size_t)m);
        }
        close(s);
    }
}

static pid_t worker[MAX];
static int worker_cnt = 0;
static time_t last = 0;

static void spawn_one_worker() {
    if (worker_cnt >= MAX) return;
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }
    if (pid == 0) {
        worker_loop();
        _exit(0);
    }
    worker[worker_cnt++] = pid;
}

static void remove_one_worker() {
    if (worker_cnt <= START) return;
    pid_t pid = worker[worker_cnt - 1];
    kill(pid, SIGTERM);
    worker_cnt--;
}

static void on_sigchld(int sig) {
    (void)sig;
    while (1) {
        pid_t p = waitpid(-1, NULL, WNOHANG);
        if (p <= 0) break;
        for (int i = 0; i < worker_cnt; ++i) {
            if (worker[i] == p) {
                for (int j = i + 1; j < worker_cnt; ++j) worker[j - 1] = worker[j];
                worker_cnt--;
                break;
            }
        }
    }
}

static void on_sigalrm(int sig) {
    (void)sig;
}

int main() {
    signal(SIGCHLD, on_sigchld);
    signal(SIGALRM, on_sigalrm);

    msqid = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    if (msqid < 0) {
        perror("msgget");
        return 1;
    }

    for (int i = 0; i < START; ++i) spawn_one_worker();

    int listen_fd = create_listen_socket(PORT);

    while (1) {
        alarm(1);
        struct sockaddr_in peer;
        socklen_t plen = sizeof(peer);
        int cli = accept(listen_fd, (struct sockaddr*)&peer, &plen);
        alarm(0);

        if (cli >= 0) {
            char line[BUF];
            if (read_line(cli, line, sizeof(line)) > 0) {
                int num = 0, cport = 0;
                char host[64];
                if (sscanf(line, "N %d PORT %d HOST %63s", &num, &cport, host) == 3) {
                    struct TaskMsg t;
                    t.mtype = 1;
                    t.num = num;
                    snprintf(t.ip, sizeof(t.ip), "%s", host);
                    t.port = cport;
                    msgsnd(msqid, &t, sizeof(t) - sizeof(long), 0);
                    last = time(NULL);

                    write_all(cli, "ok\n", 3);
                } else write_all(cli, "err\n", 4);
            }
            close(cli);
        } else {
            if (errno != EINTR && errno != EAGAIN) perror("accept");
        }
        struct msqid_ds ds;
        if (msgctl(msqid, IPC_STAT, &ds) == 0) {
            if (ds.msg_qnum > 0 && worker_cnt < MAX) {
                spawn_one_worker();
            } else if (ds.msg_qnum == 0 && worker_cnt > START) {
                if (last > 0 && (time(NULL) - last) >= IDLE_TIMEOUT) {
                    remove_one_worker();
                }
            }
        }
    }
}