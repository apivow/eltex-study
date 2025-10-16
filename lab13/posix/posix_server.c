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

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 64;
    attr.mq_curmsgs = 0;

    mqd_t mq_server_to_client = mq_open(queue_name, O_CREAT | O_WRONLY, 0666, &attr);
    if (mq_server_to_client == (mqd_t) - 1) {
        perror("mq_open s2c");
        return 1;
    }

    mqd_t mq_client_to_server = mq_open(queue_name2, O_CREAT | O_RDONLY, 0666, &attr);
    if (mq_client_to_server == (mqd_t) - 1) {
        perror("mq_open c2s");
        mq_close(mq_client_to_server);
        mq_unlink(queue_name);
        return 1;
    }

    const char *hi = "Hi!";
    if (mq_send(mq_server_to_client, hi, strlen(hi) + 1, 0) == -1) {
        perror("mq_send s2c");
        mq_close(mq_server_to_client);
        mq_close(mq_client_to_server);
        mq_unlink(queue_name);
        mq_unlink(queue_name2);
        return 1;
    }
    printf("Server send: %s\n", hi);

    char buf[64];
    unsigned int prio = 0;
    ssize_t n = mq_receive(mq_client_to_server, buf, sizeof(buf), &prio);
    if (n == -1) {
        perror("mq_receive");
        mq_close(mq_server_to_client);
        mq_close(mq_client_to_server);
        mq_unlink(queue_name);
        mq_unlink(queue_name2);
        return 1;
    }

    printf("Server received: %s\n", buf);

    mq_close(mq_server_to_client);
    mq_close(mq_client_to_server);
    mq_unlink(queue_name);
    mq_unlink(queue_name2);
    return 0;
}