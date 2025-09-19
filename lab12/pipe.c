#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        close(pipefd[1]);
        char buf[128];
        ssize_t n = read(pipefd[0], buf, sizeof(buf) - 1);
        if (n < 0) {
            perror("read");
            close(pipefd[0]);
            _exit(1);
        }
        buf[n] = '\0';
        printf("%s\n", buf);
        close(pipefd[0]);
        _exit(0);
    } else {
        close(pipefd[0]);
        const char msg[] = "Hi!";
        size_t len = strlen(msg);
        ssize_t written = write(pipefd[1], msg, (ssize_t)len);
        if (written != (ssize_t)len) perror("write");
        close(pipefd[1]);
        int status = 0;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return 1;
        }
        return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
    }
}