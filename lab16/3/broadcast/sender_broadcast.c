#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define BROADCAST_IP "255.255.255.255"
#define PORT 5000

int main() {
    int sock;
    struct sockaddr_in addr;
    int broadcast_enable = 1;
    char msg[128];

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, BROADCAST_IP, &addr.sin_addr);

    time_t now = time(NULL);
    snprintf(msg, sizeof(msg), "Time %s", ctime(&now));

    sendto(sock, msg, strlen(msg), 0, (struct sockaddr*)&addr, sizeof(addr));

    printf("Send %s\n", msg);

    close(sock);
    return 0;
}