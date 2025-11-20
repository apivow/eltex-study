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

#define SELECT 1
#define POLL 0
#define EPOLL 0

#if POLL
#include <poll.h>
#endif

#if EPOLL
#include <sys/epoll.h>
#endif

static int create_tcp_listen(uint16_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket TCP");
        exit(1);
    }
    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind tcp");
        exit(1);
    }
    if (listen(fd, 128) < 0) {
        perror("listen tcp");
        exit(1);
    }
    return fd;
}

static int create_udp_socket(uint16_t port) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket udp");
        exit(1);
    }
    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind udp");
        exit(1);
    }
    return fd;
}

static void handle_one_tcp(int lfd) {
    int cfd = accept(lfd, NULL, NULL);
    if (cfd < 0) {
        perror("accept tcp");
        return;
    }
    char buf[BUF];
    ssize_t n = read(cfd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        char out[BUF + 16];
        int m = snprintf(out, sizeof(out), "TCP %s", buf);
        if (m > 0) write(cfd, out, (size_t)m);
    }
    close(cfd);
}

static void handle_one_udp(int ufd) {
    char buf[BUF];
    struct sockaddr_in cli;
    socklen_t clen = sizeof(cli);
    ssize_t n = recvfrom(ufd, buf, sizeof(buf) - 1, 0, (struct sockaddr*)&cli, &clen);
    if (n < 0) {
        perror("recvfrom");
        return;
    }
    buf[n] = '\0';
    char out[BUF + 16];
    int m = snprintf(out, sizeof(out), "UDP %s", buf);
    if (m > 0) sendto(ufd, out, (size_t)m, 0, (struct sockaddr*)&cli, clen);
}

int main() {
    int tcp_lfd = create_tcp_listen(PORT);
    int udp_fd = create_udp_socket(PORT);

    printf("Listening TCP + UDP on %d\n", PORT);
    fflush(stdout);

#if SELECT
    while (1) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(tcp_lfd, &rfds);
        FD_SET(udp_fd, &rfds);
        int max_fd = tcp_lfd > udp_fd ? tcp_lfd : udp_fd;

        int rc = select(max_fd + 1, &rfds, NULL, NULL, NULL);
        if (rc < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }
        if (FD_ISSET(tcp_lfd, &rfds)) handle_one_tcp(tcp_lfd);
        if (FD_ISSET(udp_fd, &rfds)) handle_one_udp(udp_fd);
    }
#endif

#if POLL
    struct pollfd fds[2];
    fds[0].fd = tcp_lfd;
    fds[0].events = POLLIN;
    fds[1].fd = udp_fd;
    fds[1].events = POLLIN;

    while (1) {
        int rc = poll(fds, 2, -1);
        if (rc < 0) {
            if (errno == EINTR) continue;
            perror("poll");
            break;
        }
        if (fds[0].revents & POLLIN) handle_one_tcp(tcp_lfd);
        if (fds[1].revents & POLLIN) handle_one_udp(udp_fd);
    }
#endif

#if EPOLL
    int ep = epoll_create1(0);
    if (ep < 0) {
        perror("epoll create");
        exit(1);
    }
    struct epoll_event ev, evs[4];
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.u32 = 1;
    epoll_ctl(ep, EPOLL_CTL_ADD, tcp_lfd, &ev);
    ev.events = EPOLLIN;
    ev.data.u32 = 2;
    epol_ctl(ep, EPOLL_CTL_ADD, udp_fd, &ev);

    while (1) {
        int n = epoll_wait(ep, evs, 4, -1);
        if (n < 0) {
            if (errno ==EINTR) continue;
            perror("epol_wait");
            break;
        }
        for (int i = 0; i < n; ++i) {
            if (evs[i].data.u32 == 1) handle_one_tcp(tcp_lfd);
            else if (evs[i].data.u32 == 2) handle_one_udp(udp_fd);
        }
    }
#endif

    close(tcp_lfd);
    close(udp_fd);
    return 0;
}