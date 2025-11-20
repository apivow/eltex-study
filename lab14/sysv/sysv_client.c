#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#define TEXT_SIZE 128

typedef struct {
    char text[TEXT_SIZE];
} ShmData;

static int sem_op(int semid, unsigned short semnum, short delta) {
    struct sembuf op = {semnum, delta, 0};
    return semop(semid, &op, 1);
}

int main() {
    key_t key_shm = ftok(".", 65);
    key_t key_sem = ftok(".", 66);
    if (key_shm == -1 || key_sem == -1) {
        perror("ftok");
        return 1;
    }

    int shmid = shmget(key_shm, sizeof(ShmData), 0600);
    if(shmid == -1) {
        perror("shmget");
        return 1;
    }

    ShmData *data = (ShmData*)shmat(shmid, NULL, 0);
    if (data == (void*) - 1) {
        perror("shmat");
        return 1;
    }

    int semid = semget(key_sem, 2, 0600);
    if (semid == -1) {
        perror("semget");
        return 1;
    }

    if (sem_op(semid, 0, -1) == -1) perror("sem_op c2s");

    printf("Client got: %s\n", data->text);

    strncpy(data->text, "Hello!", TEXT_SIZE - 1);
    data->text[TEXT_SIZE - 1] = '\0';
    if(sem_op(semid, 1, +1) == -1) perror("sem_op c2s");

    shmdt(data);
    return 0;
}