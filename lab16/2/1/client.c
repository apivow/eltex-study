#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
 
#define PORT 7777
#define BUF 1024

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        return 1;
    }

    int s = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, argv[1], &addr.sin_addr) != 1) {
        perror("inet_pton");
        exit(1);
    }

    if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        exit(1);
    }

    dprintf(s, "%s\n", argv[2]);

    char buf[BUF];
    ssize_t n;
    while ((n = read(s, buf, sizeof(buf))) > 0) write(STDOUT_FILENO, buf, n);

    close(s);
    return 0;
}