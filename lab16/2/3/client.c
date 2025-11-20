#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 7777
#define BUF 1024

static int make_listen_any(uint16_t port) {
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
    if (listen(fd, 8) < 0) {
        perror("listen");
        exit(1);
    }
    return fd;
}

static uint16_t get_port(int fd) {
    struct sockaddr_in addr;
    socklen_t alen = sizeof(addr);
    if (getsockname(fd, (struct sockaddr*)&addr, &alen) < 0) {
        perror("getsockname");
        exit(1);
    }
    return ntohs(addr.sin_port);
}

static int connect_ip(const char *ip, uint16_t port) {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1) {
        fprintf(stderr, "bad ip\n");
        exit(1);
    }
    if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        exit(1);
    } 
    return s;
}

static ssize_t write_all(int fd, const void *buf, size_t len) {
    const char *p = (const char*)buf;
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

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "use %s", argv[0]);
        return 1;
    }
    int N = atoi(argv[1]);
    const char *my_ip = argv[2];

    int cb = make_listen_any(0);
    uint16_t cb_port = get_port(cb);
    fprintf(stderr, "Client listen port %u\n", (unsigned)cb_port); //

    int s = connect_ip(my_ip, PORT);
    char out[BUF];
    int m = snprintf(out, sizeof(out), "N %d port %u host %s\n", N, (unsigned)cb_port, my_ip);
    write_all(s, out, (size_t)m);
    char tmp[64];
    read_line(s, tmp, sizeof(tmp));
    close(s);

    int c = accept(cb, NULL, NULL);
    if (c < 0) {
        perror("accept");
        return 1;
    }
    char resp[BUF];
    if (read_line(c, resp, sizeof(resp)) > 0) fputs(resp, stdout);
    close(c);
    close(cb);
    return 0;
}