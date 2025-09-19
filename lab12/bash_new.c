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

        int comand = 4, comand_cnt = 0;
        char **comand_str = (char**)malloc((size_t)comand * sizeof(char *));
        if (!comand_str) {
            perror("malloc comand_str");
            break;
        }

        char *seg = strtok(line, "|");
        while(seg) {
            while (*seg == ' ' || *seg == '\t') seg++;
            if (*seg != '\0') {
                if (comand_cnt >= comand) {
                    comand *= 2;
                    char **tmp = (char**)realloc(comand_str, (size_t)comand * sizeof(char *));
                    if(!tmp) {
                        perror("realloc tmp comand_str");
                        free(comand_str);
                        comand_str = NULL;
                        break;
                    }
                    comand_str = tmp;
                }
                comand_str[comand_cnt++] = seg;
            }
            seg = strtok(NULL, "|");
        }

        if(!comand_str) break;
        if(comand_cnt == 0) {
            free(comand_str);
            continue;
        }

        char ***comand_argvs = (char ***)malloc((size_t)comand_cnt * sizeof(char **));
        if (!comand_argvs) {
            perror("malloc command_argvs");
            free(comand_str);
            break;
        }

        int ok = 1;
        for (int i = 0; i < comand_cnt && ok; i++) {
            int argc = 0, argv_size = 8;
            char **argv = (char **)malloc((size_t)argv_size * sizeof(char *));
            if (!argv) {
                perror("malloc argv");
                ok = 0;
                break;
            }

            char *tok = strtok(comand_str[i], " \t");
            while(tok) {
                if (argc + 1 >= argv_size) {
                    argv_size *= 2;
                    char **tmp = (char **)realloc(argv, (size_t)argv_size * sizeof(char *));
                    if (!tmp) {
                        perror("realloc argv");
                        free(argv);
                        argv = NULL;
                        ok =0;
                        break;
                    }
                    argv = tmp;
                }
                argv[argc++] = tok;
                tok = strtok(NULL, " \t");
            }
            if(!argv) break;
            argv[argc] = NULL;

            if (argc == 0) {
                free(argv);
                ok = 0;
                break;
            }
            comand_argvs[i] = argv;
        }
        if (!ok) {
            for (int j = 0; j < comand_cnt; j++) if (comand_argvs[j]) free(comand_argvs[j]);
            free(comand_argvs);
            free(comand_str);
            continue;
        }

        if (comand_cnt == 1 && strcmp(comand_argvs[0][0], "exit") == 0) {
            free(comand_argvs[0]);
            free(comand_argvs);
            free(comand_str);
            break;
        }

        int num_pipes = comand_cnt - 1;
        int (*pipes)[2] = NULL;
        if (num_pipes > 0) {
            pipes = (int (*)[2])malloc((size_t)num_pipes * sizeof(int[2]));
            if (!pipes) {
                perror("malloc pipes");
                for (int i = 0; i < comand_cnt; i++) free(comand_argvs[i]);
                free(comand_argvs);
                free(comand_str);
                continue;
            }

            int pipe_ok = 1;
            for (int i = 0; i < num_pipes; i++) {
                if (pipe(pipes[i]) == -1) {
                    perror("pipe");
                    for(int j = 0; j < i; j++) {
                        close(pipes[j][0]);
                        close(pipes[j][1]);
                    }
                    free(pipes);
                    pipes = NULL;
                    pipe_ok = 0;
                    break;
                }
            }
            if(!pipe_ok) {
                for (int i = 0; i < comand_cnt; i++) free(comand_argvs[i]);
                free(comand_argvs);
                free(comand_str);
                continue;
            }
        }

        pid_t *pids = (pid_t *)malloc((size_t)comand_cnt * sizeof(pid_t));
        if(!pids) {
            perror("malloc pids");
            if(pipes) {
                for (int i = 0; i < num_pipes; i++){
                    close(pipes[i][0]);
                    close(pipes[i][1]);
                }
                free(pipes);
            }
            for (int i = 0; i < comand_cnt; i++) free(comand_argvs[i]);
            free(comand_argvs);
            free(comand_str);
            continue;
        }

        int fork_failed = 0;
        for(int i = 0; i < comand_cnt; i++){
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                fork_failed = 1;
                break;
            }
            if(pid == 0) {
                if (pipes) {
                    if (i > 0) {
                        if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1) {
                            perror("dup2 in");
                            _exit(126);
                        }
                    }
                    if (i < comand_cnt - 1) {
                        if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                            perror("dup2 out");
                            _exit(126);
                        }
                    }
                    for(int j = 0; j < num_pipes; j++){
                        close(pipes[j][0]);
                        close(pipes[j][1]);
                    }
                }
                execvp(comand_argvs[i][0], comand_argvs[i]);
                perror("execvp");
                _exit(127);
            } else {
                pids[i] = pid;
            }
        }
        if (pipes) {
            for (int i = 0; i < num_pipes; i++) {
                close(pipes[i][0]);
                close(pipes[i][1]);
            }
            free(pipes);
        }

        if(fork_failed) {
            for (int i = 0; i < comand_cnt; i++) if (pids[i] > 0) waitpid(pids[i], NULL, 0);
            free(pids);
            for(int i = 0; i < comand_cnt; i++) free(comand_argvs[i]);
            free(comand_argvs);
            free(comand_str);
            continue;
        }

        for (int i = 0; i < comand_cnt; i ++) waitpid(pids[i], NULL, 0);

        free(pids);
        for(int i = 0; i < comand_cnt; i++) free(comand_argvs[i]);
        free(comand_argvs);
        free(comand_str);
    }

    free(line);
    return 0;
}