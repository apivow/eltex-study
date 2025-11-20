#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 7777
#define BUF 1024

static int connect_ip(const char *ip, int port) {
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

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s <ip> <num>\n", argv[0]);
        return 1;
    }
    const char *ip = argv[1];
    const char *num = argv[2];

    int d = connect_ip(ip, PORT);

    char line[BUF];
    if (read_line(d, line, sizeof(line)) <= 0) { 
        perror("read"); 
        return 1; 
    }
    close(d);

    int port = 0;
    if (strncmp(line, "BUSY", 4) == 0) {
        fprintf(stderr, "BUSY\n");
        return 1;
    }
    if (sscanf(line, "PORT %d", &port) != 1 || port <= 0 || port > 65535) {
        fprintf(stderr, "bad reply: %s\n", line);
        return 1;
    }

    int w = connect_ip(ip, port);
    char out[BUF];
    int m = snprintf(out, sizeof(out), "N %s\n", num);
    if (m < 0 || (size_t)m >= sizeof(out)) return 1;
    if (write_all(w, out, (size_t)m) < 0) { 
        perror("write"); 
        return 1; 
    }

    char resp[BUF];
    if (read_line(w, resp, sizeof(resp)) > 0) fputs(resp, stdout);
    close(w);
    return 0;
}