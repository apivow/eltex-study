#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/local_stream_socket"

int main() {
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char buf[1024] = {0};

    unlink(SOCKET_PATH);
    server_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    addr.sun_family = AF_LOCAL;
    strcpy(addr.sun_path, SOCKET_PATH);

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 1);
    printf("Server waiting %s\n", SOCKET_PATH);

    client_fd = accept(server_fd, NULL, NULL);
    read(client_fd, buf, sizeof(buf));
    printf("Received: %s\n", buf);

    write(client_fd, "Hi", 3);

    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}