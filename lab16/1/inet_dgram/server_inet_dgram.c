#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sock;
    struct sockaddr_in addr, client;
    socklen_t len = sizeof(client);
    char buf[1024] = {0};

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(5556);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(sock, (struct sockaddr*)&addr, sizeof(addr));

    recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*)&client, &len);
    printf("Received: %s\n", buf);

    sendto(sock, "Hi", 3, 0, (struct sockaddr*)&client, len);

    close(sock);
    return 0;
}