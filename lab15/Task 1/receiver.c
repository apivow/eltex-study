#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

void sig_handler(int sig_num, siginfo_t *info, void *args) {
    int *param = (int *)args;
    printf("Signal SIGUSR1 - %d %d %d\n", sig_num, *param, info->si_signo);
}

int main() {
    struct sigaction handler;
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    handler.sa_sigaction = sig_handler;
    handler.sa_flags = 0;
    handler.sa_mask = set;

    if (sigaction(SIGUSR1, &handler, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    printf("PID: %d\n", getpid());
    printf("SIGUSR1.....\n");

    while(1) {
        sleep(1);
    }
}