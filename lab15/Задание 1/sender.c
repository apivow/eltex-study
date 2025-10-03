#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Use: %s <PID>\n", argv[0]);
        return 1;
    }

    char* end = NULL;
    long pid_long = strtol(argv[1], &end, 10);
    if (*argv[1] == '\0' || *end != '\0' || pid_long <= 0) {
        fprintf(stderr, "Bad PID: %s\n", argv[1]);
        return 1;
    }

    if (kill((pid_t)pid_long, SIGUSR1) == -1) {
        perror("kill");
        return 1;
    }

    printf("SIGUSR1 sent to %ld\n", pid_long);
    return 0;
}