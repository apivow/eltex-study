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
    int qid = msgget(key, IPC_CREAT | 0666);
    if (qid == -1) {
        perror("msgget");
        return 1;
    }

    struct msgbuf msg;
    msg.mtype = 1;
    snprintf(msg.mtext, sizeof(msg.mtext), "%s", "Hi!");
    if (msgsnd(qid, &msg, strlen(msg.mtext) + 1, 0) == -1) {
        perror("msgsnd");
        msgctl(qid, IPC_RMID, NULL);
        return 1;
    }
    printf("Server send: %s\n", msg.mtext);

    struct msgbuf reply;
    if (msgrcv(qid, &reply, sizeof(reply.mtext), 2, 0) == -1) {
        perror("msgrcv");
        msgctl(qid, IPC_RMID, NULL);
        return 1;
    }
    printf("Server received: %s\n", reply.mtext);
    
    if (msgctl(qid, IPC_RMID, NULL) == -1) {
        perror("msgctl IPC_RMID");
        return 1;
    }
    return 0;
}