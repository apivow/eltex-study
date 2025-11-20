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

    unlink(SERVER_PATH);
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    server.sun_family = AF_LOCAL;
    strcpy(server.sun_path, SERVER_PATH);

    bind(sock, (struct sockaddr*)&server, sizeof(server));

    socklen_t len = sizeof(client);
    recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*)&client, &len);
    printf("Received: %s\n", buf);

    sendto(sock, "Hi", 3, 0, (struct sockaddr*)&client, len);
    
    close(sock);
    unlink(SERVER_PATH);
    return 0;
}