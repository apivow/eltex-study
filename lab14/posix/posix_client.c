#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>

#define SHM_NAME "/my_sem_shm"
#define SEM_SERVER_TO_CLIENT "/sem_s2c"
#define SEM_CLIENT_TO_SERVER "/sem_c2s"
#define TEXT_SIZE 128

typedef struct  {
    char text[TEXT_SIZE];
} ShmData;

int main() {
    int fd = shm_open(SHM_NAME, O_RDWR, 0);
    if (fd == -1) {
        perror("shm_open");
        return 1;
    }

    ShmData *data = (ShmData*)mmap(NULL, sizeof(ShmData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    sem_t *server_to_client = sem_open(SEM_SERVER_TO_CLIENT, 0);
    if (server_to_client == SEM_FAILED) {
        perror("sep_open s2c");
        munmap(data, sizeof(ShmData));
        close(fd);
        return 1;
    }
    sem_t *client_to_server = sem_open(SEM_CLIENT_TO_SERVER, 0);
    if(client_to_server == SEM_FAILED) {
        perror("sem_open c2s");
        sem_close(server_to_client);
        munmap(data, sizeof(ShmData));
        close(fd);
        return 1;
    }

    sem_wait(server_to_client);
    printf("client got: %s\n", data->text);

    strncpy(data->text, "Hello!", TEXT_SIZE - 1);
    data->text[TEXT_SIZE -1] = '\0';
    sem_post(client_to_server);

    sem_close(server_to_client);
    sem_close(client_to_server);
    munmap(data, sizeof(ShmData));
    close(fd);
    return 0;
}
