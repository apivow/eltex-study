#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int server_fd, client_fd;
    struct sockaddr_in addr;
    char buf[1024] = {0};

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(5555);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    listen(server_fd, 1);
    printf("server waiting...\n");

    client_fd = accept(server_fd, NULL, NULL);
    read(client_fd, buf, sizeof(buf));
    printf("Received: %s\n", buf);

    write(client_fd, "Hi", 3);

    close(client_fd);
    close(server_fd);
    return 0;
}