#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 7777
#define BUF 1024

static void to_upper(char *s, ssize_t n) {
    for ( ssize_t i = 0; i < n; ++i) if (islower((unsigned char)s[i])) s[i] = (char)toupper(s[i]);
}

int main() {
    signal(SIGCHLD, SIG_IGN);

    int lf = socket(AF_INET, SOCK_STREAM, 0);
    int one = 1;
    setsockopt(lf, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(lf, (struct sockaddr *)&addr, (socklen_t)sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }
    if (listen(lf, 128) < 0) {
        perror("listen");
        exit(1);
    }
    printf("Listening on %d\n", PORT);

    while(1) {
        int cf =accept(lf, NULL, NULL);
        if (cf < 0) {
            perror("accept");
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(cf);
            continue;
        }
        if (pid == 0) {
            close(lf);
            char buf[BUF];
            ssize_t n = read(cf, buf, sizeof(buf));
            if (n > 0) {
                printf("pid = %d: got %zd bytes: '%.*s'\n", getpid(), n, (int)n, buf);
                fflush(stdout);
                to_upper(buf, n);
                write(cf, "PROCESSED: ", 11);
                write(cf, buf, n);
            }
            close(cf);
            _exit(0);
        } else close(cf);
    }
}