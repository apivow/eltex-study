#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MCAST_GRP "239.0.0.1"
#define MCAST_PORT 6000

int main() {
    int sock;
    struct sockaddr_in addr;
    struct ip_mreq mreq;
    char buf[256];

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(MCAST_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    mreq.imr_multiaddr.s_addr = inet_addr(MCAST_GRP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    printf("Listening... %s %d\n", MCAST_GRP, MCAST_PORT);

    recvfrom(sock, buf, sizeof(buf) - 1, 0, NULL, NULL);
    buf[255] = '0';
    printf("Received %s\n", buf);

    close(sock);
    return 0;
}