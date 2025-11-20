#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sock;
    struct sockaddr_in addr;
    char buf[1024] = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(5555);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    connect(sock, (struct sockaddr*)&addr, sizeof(addr));

    write(sock, "Hello", 6);
    read(sock, buf, sizeof(buf));
    printf("Received: %s\n", buf);

    close(sock);
    return 0;
}