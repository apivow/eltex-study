#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5000

int main() {
    int sock;
    struct sockaddr_in addr;
    char buf[256];
    socklen_t addrlen = sizeof(addr);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    printf("Waiting...\n");
    recvfrom(sock, buf, sizeof(buf) - 1, 0, NULL, NULL);
    buf[255] = '0';
    printf("received %s\n", buf);

    close(sock);
    return 0;
}