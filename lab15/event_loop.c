#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main() {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);

    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
        perror("sigprocmask");
        return 1;
    }

    printf("PID: %ld\n", (long)getpid());
    printf("Wait SIGUSR1 from sigwait\n");

    while(1) {
        int sig = 0;
        int ret = sigwait(&set, &sig);
        if (ret == 0) {
            printf("Signal: %s (%d)\n",
                    (sig == SIGUSR1 ? "SIGUSR1" : "other"), sig);
        } else {
            fprintf(stderr, "sigwait: %s\n", strerror(ret));
            return 1;
        }
    }
}