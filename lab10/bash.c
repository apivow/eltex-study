#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    char *line = NULL;
    size_t size = 0;

    while(1) {
        printf(">>>>>> ");
        fflush(stdout);

        ssize_t len = getline(&line, &size, stdin);
        if(len < 0) break;

        if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';

        char *tok = strtok(line, " \t");
        if (!tok) continue;

        int argc = 0, argv_size = 8;
        char **argv = (char **)malloc(argv_size * sizeof(char*));
        if (!argv) {
            perror("malloc");
            break;
        }

        argv[argc++] = tok;
        while((tok = strtok(NULL, " \t")) != NULL) {
            if (argc + 1 >= argv_size) {
                argv_size *= 2;
                char **tmp = (char**)realloc(argv, argv_size * sizeof(char*));
                if (!tmp) {
                    perror("realloc");
                    free(argv);
                    argv = NULL;
                    break;
                }
                argv = tmp;
            }
            argv[argc++] = tok;
        }
        if(!argv) break;
        argv[argc] = NULL;

        if (strcmp(argv[0], "exit") == 0) {
            free(argv);
            break;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            free(argv);
            continue;
        }

        if (pid == 0) {
            execvp(argv[0], argv);
            perror("execvp");
            _exit(127);
        } else {
            int status = 0;
            if (waitpid(pid, &status, 0) < 0) {
                perror("waitpid");
            }
        }
        free(argv);
    }
    free(line);
    return 0;
}