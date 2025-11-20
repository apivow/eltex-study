#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define MCAST_GPR "239.0.0.1"
#define MCAST_PORT 6000

int main() {
    int sock;
    struct sockaddr_in addr;
    char msg[128];

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(MCAST_PORT);
    inet_pton(AF_INET, MCAST_GPR, &addr.sin_addr);

    time_t now = time(NULL);
    snprintf(msg, sizeof(msg), "Time %s", ctime(&now));

    if (sendto(sock, msg, strlen(msg), 0, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("sendto");
        exit(1);
    }

    printf("Sent %s\n", msg);

    close(sock);
    return 0;
}