#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <errno.h>

#define TEXT_SIZE 128

typedef struct {
    char text[TEXT_SIZE];
} ShmData;

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

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

    int shmid = shmget(key_shm, sizeof(ShmData), IPC_CREAT | 0600);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    ShmData *data = (ShmData*)shmat(shmid, NULL, 0);
    if (data == (void*) - 1) {
        perror("shmat");
        shmctl(shmid, IPC_RMID, NULL);
        return 1;
    }

    int semid = semget(key_sem, 2, IPC_CREAT | 0600);
    if (semid == -1) {
        perror("semget");
        shmdt(data);
        shmctl(shmid, IPC_RMID, NULL);
        return 1;
    }

    unsigned short init_vals[2] = {0, 0};
    union semun arg;
    arg.array = init_vals;
    if (semctl(semid, 0, SETALL, init_vals) == -1) {
        perror("semctl SETALL");
        shmdt(data);
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID);
        return 1;
    }

    strncpy(data->text, "Hi!", TEXT_SIZE - 1);
    data->text[TEXT_SIZE - 1] = '\0';
    printf("Server wrote: %s\n", data->text);
    if (sem_op(semid, 0, +1) == -1) perror("sem_op s2c");
    if (sem_op(semid, 1, -1) == -1) perror("sem_op c2s");

    printf("Server got reply: %s\n", data->text);

    shmdt(data);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);

    return 0;
} 