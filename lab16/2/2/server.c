#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

struct Worker {
    pid_t pid;
    int notify_fd;
    int port;
    int busy;
    time_t last_free_ts;
};

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

static ssize_t write_all(int fd, const void *buf, size_t len) {
    const char *p = (const char*)buf;
    size_t left = len;
    while (left > 0) {
        ssize_t n = write(fd, p, left);
        if (n < 0) { if (errno == EINTR) continue; 
            return -1; 
        }
        p += n;
        left -= (size_t)n;
    }
    return (ssize_t)len;
}

static ssize_t read_line(int fd, char *buf, size_t cap) {
    size_t i = 0;
    while (i + 1 < cap) {
        char c;
        ssize_t n = read(fd, &c, 1);
        if (n == 0) break;
        if (n < 0) { if (errno == EINTR) continue; 
            return -1; 
        }
        buf[i++] = c;
        if (c == '\n') break;
    }
    buf[i] = '\0';
    return (ssize_t)i;
}

static long long sum_1_to_n(long long n) {
    if (n < 0) return 0;
    return (n * (n + 1) / 2);
}

static void worker_process(int notify_fd) {
    int srv = create_listen_socket(0);

    struct sockaddr_in addr;
    socklen_t alen = sizeof(addr);
    if (getsockname(srv, (struct sockaddr*)&addr, &alen) < 0) { 
        perror("getsockname"); 
        exit(1); 
    }
    int port = ntohs(addr.sin_port);

    char buf[64];
    int m = snprintf(buf, sizeof(buf), "PORT %d\n", port);
    write_all(notify_fd, buf, (size_t)m);
    write_all(notify_fd, "FREE\n", 5);

    while (1) {
        int cf = accept(srv, NULL, NULL);
        if (cf < 0) { if (errno == EINTR) continue; 
            perror("accept"); 
            exit(1); 
        }

        char line[BUF];
        if (read_line(cf, line, sizeof(line)) > 0) {
            long long num = 0;
            if (sscanf(line, "N %lld", &num) == 1) {
                long long res = sum_1_to_n(num);
                char out[64];
                int k = snprintf(out, sizeof(out), "Res %lld\n", res);
                write_all(cf, out, (size_t)k);
            } else {
                write_all(cf, "ERR\n", 4);
            }
        }
        close(cf);
        write_all(notify_fd, "FREE\n", 5);
    }
}

static void on_sigchld(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
}

static void set_nonblock(int fd) {
    int fl = fcntl(fd, F_GETFL, 0);
    if (fl >= 0) fcntl(fd, F_SETFL, fl | O_NONBLOCK);
}

static int spawn_worker(struct Worker *w) {
    int pfd[2];
    if (pipe(pfd) < 0) { 
        perror("pipe"); 
        exit(1); 
    }

    pid_t pid = fork();
    if (pid < 0) { 
        perror("fork"); 
        exit(1); 
    }

    if (pid == 0) {
        close(pfd[0]);
        worker_process(pfd[1]);
        _exit(0);
    } else {
        close(pfd[1]);
        w->pid = pid;
        w->notify_fd = pfd[0];
        w->port = 0;
        w->busy = 1;
        w->last_free_ts = time(NULL);

        char buf[BUF];
        if (read_line(w->notify_fd, buf, sizeof(buf)) <= 0) return -1;
        int p = 0;
        if (sscanf(buf, "PORT %d", &p) != 1 || p <= 0) return -1;
        w->port = p;

        set_nonblock(w->notify_fd);
        return 0;
    }
}

static void drain_notify(struct Worker *w) {
    char buf[128];
    while (1) {
        ssize_t n = read(w->notify_fd, buf, sizeof(buf));
        if (n < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            break;
        }
        if (n == 0) break;
        if (memmem(buf, (size_t)n, "FREE", 4) != NULL) {
            w->busy = 0;
            w->last_free_ts = time(NULL);
        }
    }
}

int main() {
    signal(SIGCHLD, on_sigchld);

    int listen_fd = create_listen_socket(PORT);

    struct Worker worker[MAX];
    int worker_cnt = 0;

    for (int i = 0; i < START && worker_cnt < MAX; ++i) {
        if (spawn_worker(&worker[worker_cnt]) == 0) worker_cnt++;
    }

    while (1) {
        int cli = accept(listen_fd, NULL, NULL);
        if (cli < 0) { if (errno == EINTR) continue; 
            perror("accept"); 
            continue; 
        }

        for (int i = 0; i < worker_cnt; ++i) drain_notify(&worker[i]);

        int idx = -1;
        for (int i = 0; i < worker_cnt; ++i) {
            if (!worker[i].busy && worker[i].port > 0) { 
                idx = i; 
                break; 
            }
        }
        if (idx < 0 && worker_cnt < MAX) {
            if (spawn_worker(&worker[worker_cnt]) == 0) {
                worker_cnt++;
            }
        }

        if (idx >= 0) {
            worker[idx].busy = 1;
            char msg[64];
            int m = snprintf(msg, sizeof(msg), "PORT %d\n", worker[idx].port);
            write_all(cli, msg, (size_t)m);
        } else {
            write_all(cli, "BUSY\n", 5);
        }
        close(cli);

        for (int i = 0; i < worker_cnt; ) {
            int remove_it = (worker_cnt > START) &&
                            (!worker[i].busy) &&
                            ((time(NULL) - worker[i].last_free_ts) > IDLE_TIMEOUT);
            if (remove_it) {
                kill(worker[i].pid, SIGTERM);
                close(worker[i].notify_fd);
                for (int j = i + 1; j < worker_cnt; ++j) worker[j - 1] = worker[j];
                worker_cnt--;
                continue;
            }
            ++i;
        }
    }
}