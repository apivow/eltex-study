#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SERVER_PATH "/tmp/local_dgram_server"
#define CLIENT_PATH "/tmp/local_dgram_client"

int main() {
    int sock;
    struct sockaddr_un server, client;
    char buf[1024] = {0};

    unlink(CLIENT_PATH);
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    client.sun_family = AF_LOCAL;
    strcpy(client.sun_path, CLIENT_PATH);
    bind(sock, (struct sockaddr*)&client, sizeof(client));

    server.sun_family = AF_LOCAL;
    strcpy(server.sun_path, SERVER_PATH);

    sendto(sock, "Hello", 6, 0, (struct sockaddr*)&server, sizeof(server));
    recvfrom(sock, buf, sizeof(buf), 0, NULL, NULL);
    printf("Received: %s\n", buf);

    close(sock);
    unlink(CLIENT_PATH);
    return 0;
}