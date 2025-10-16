#define _GNU_SOURCE
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main() {

    const char *queue_name = "/my_queue_example"; //s2c
    const char *queue_name2 = "/my_queue_example2"; //c2s

    mqd_t mq_server_to_client, mq_client_to_server;
    while(1) {
        mq_server_to_client = mq_open(queue_name, O_RDONLY);
        if (mq_server_to_client != (mqd_t) - 1) break;
        if (errno != ENOENT) {
            perror("mq_open s2c");
            return 1;
        }
        usleep(100 * 1000);
    }

    while(1) {
        mq_client_to_server = mq_open(queue_name2, O_WRONLY);
        if (mq_client_to_server != (mqd_t) - 1) break;
        if (errno != ENOENT) {
            perror("mq_open c2s");
            mq_close(mq_server_to_client);
            return 1;
        }
        usleep(100 * 1000);
    }

    char buf[64];
    unsigned int prio = 0;
    ssize_t n = mq_receive(mq_server_to_client, buf, sizeof(buf), &prio);
    if (n == -1) {
        perror("mq_reseive s2c");
        mq_close(mq_server_to_client);
        mq_close(mq_client_to_server);
        return 1;
    }
    printf("Client received: %s\n", buf);

    const char *hello = "Hello!";
    if (mq_send(mq_client_to_server, hello, strlen(hello) + 1,0) == -1) {
        perror("mq_send c2s");
        mq_close(mq_server_to_client);
        mq_close(mq_client_to_server);
        return 1;
    }
    printf("Client send: %s\n" , hello);

    mq_close(mq_server_to_client);
    mq_close(mq_client_to_server);
    return 0;
}