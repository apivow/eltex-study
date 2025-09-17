#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <locale.h>

int main() {
    setlocale(LC_ALL, "Russia");

    printf("Текущий процесс: pid = %d, ppid = %d\n", getpid(), getppid());

    pid_t p1 = fork();
    if (p1 < 0) {
        perror("fork p1");
        return 1;
    }

    if (p1 == 0) {
        printf("Процесс 1: pid = %d, ppid = %d\n", getpid(), getppid());
        pid_t p3 = fork();
        if (p3 < 0) {
            perror("fork p3");
            _exit(1);
        }
        if(p3 == 0) {
            printf("Процесс 3: pid = %d, ppid = %d\n", getpid(), getppid());
            _exit(0);
        }
        pid_t p4 = fork();
        if (p4 < 0) {
            perror("fork p4");
            _exit(1);
        }
        if(p4 == 0) {
            printf("Процесс 4: pid = %d, ppid = %d\n", getpid(), getppid());
            _exit(0);
        }
        int status;
        while (wait(&status) > 0) {}
        _exit(0);
    }

    pid_t p2 = fork();
    if (p2 < 0) {
        perror("fork p2");
        int status;
        waitpid(p1, &status, 0);
        return 1;
    }

    if (p2 == 0) {
        printf("Процесс 2: pid = %d, ppid = %d\n", getpid(), getppid());
        pid_t p5 = fork();
        if (p5 < 0) {
            perror("fork p5");
            _exit(1);
        }
        if(p5 == 0) {
            printf("Процесс 5: pid = %d, ppid = %d\n", getpid(), getppid());
            _exit(0);
        }
        int status;
        waitpid(p5, &status, 0);
        _exit(0);
    }
    int status;
    waitpid(p1, &status, 0);
    waitpid(p2, &status, 0);
    return 0;
}