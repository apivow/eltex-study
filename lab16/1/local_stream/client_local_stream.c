#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/local_stream_socket"

int main() {
    int sock;
    struct sockaddr_un addr;
    char buf[1024] = {0};

    sock = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    addr.sun_family = AF_LOCAL;
    strcpy(addr.sun_path, SOCKET_PATH);

    connect(sock, (struct sockaddr*)&addr, sizeof(addr));

    write(sock, "Hello", 6);
    read(sock, buf, sizeof(buf));
    printf("Received: %s\n", buf);

    close(sock);
    return 0;
}