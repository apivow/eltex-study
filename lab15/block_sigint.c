#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int main() {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);

    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
        perror("sigprocmask");
        return 1;
    }

    printf("PID: %ld\n", (long)getpid());
    printf("SIGINT block\n");

    while(1) {
        sleep(1);
    }
}