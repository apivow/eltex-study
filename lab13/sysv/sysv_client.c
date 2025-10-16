#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

struct msgbuf {
    long mtype;
    char mtext[64];
};

int main() {

    key_t key = 0x12345;
    int qid;
    while(1) {
        qid = msgget(key, 0666);
        if (qid != -1) break;
        if (errno != ENOENT) {
            perror("msgget");
            return 1;
        }
        usleep(100 * 1000);
    }

    struct msgbuf msg;
    if (msgrcv(qid, &msg, sizeof(msg.mtext), 1, 0) == -1) {
        perror("msgrcv");
        return 1;
    }
    printf("Client received: %s\n", msg.mtext);

    struct msgbuf reply;
    reply.mtype = 2;
    snprintf(reply.mtext, sizeof(reply.mtext), "%s", "Hello!");
    if (msgsnd(qid, &reply, strlen(reply.mtext) + 1, 0) == -1) {
        perror("msgsnd");
        return 1;
    }
    printf("Client send: %s\n", reply.mtext);
    return 0;
}