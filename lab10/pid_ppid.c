#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <locale.h>

int main() {
    setlocale(LC_ALL, "Russia");
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("Ребенок: pid = %d, ppid = %d\n", getpid(), getppid());
        _exit(42);
    } else {
        printf("Родитель: pid = %d, ppid = %d, ребенок = %d\n", getpid(), getppid(), pid);
        int status = 0;
        pid_t w = waitpid(pid, &status, 0);
        if (w == -1) { 
            perror("waitpid");
            return 1;
        }
        if (WIFEXITED(status)) {
            printf("Ребенок %d завершился с кодом %d\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Ребенок %d завершился сигналом %d\n", pid, WTERMSIG(status));
        } else {
            printf("Ребенок %d завершился со статусом 0x%x\n", pid, status);
        }
    }
    return 0;
}