#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sock;
    struct sockaddr_in addr;
    char buf[1024] = {0};
    socklen_t len = sizeof(addr);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(5556);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    sendto(sock, "Hello", 6, 0, (struct sockaddr*)&addr, len);
    recvfrom(sock, buf, sizeof(buf), 0, NULL, NULL);
    printf("Received: %s\n", buf);

    close(sock);
    return 0;
}