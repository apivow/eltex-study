#include <fcntl.h> 
#include <sys/mman.h> 
#include <sys/stat.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <semaphore.h>

#define SHM_NAME "/my_sem_shm"
#define SEM_SERVER_TO_CLIENT "/sem_s2c"
#define SEM_CLIENT_TO_SERVER "/sem_c2s"
#define TEXT_SIZE 128

typedef struct {
    char text[TEXT_SIZE];
} ShmData;

int main() {
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
    if (fd == -1) {
        perror("shm_open");
        return 1;
    }
    if (ftruncate(fd, sizeof(ShmData)) == -1) {
        perror("ftruncate");
        close(fd);
        shm_unlink(SHM_NAME);
        return 1;
    }

    ShmData *data = (ShmData*)mmap(NULL, sizeof(ShmData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        shm_unlink(SHM_NAME);
        return 1;
    }

    sem_t *server_to_client = sem_open(SEM_SERVER_TO_CLIENT, O_CREAT, 0600, 0);
    if (server_to_client == SEM_FAILED) {
        perror("sem_open s2c");
        munmap(data, sizeof(ShmData));
        close(fd);
        shm_unlink(SHM_NAME);
        return 1;
    }

    sem_t *client_to_server = sem_open(SEM_CLIENT_TO_SERVER, O_CREAT, 0600, 0);
    if (client_to_server == SEM_FAILED) {
        perror("sem_open c2s");
        sem_close(server_to_client);
        sem_unlink(SEM_SERVER_TO_CLIENT);
        munmap(data, sizeof(ShmData));
        close(fd);
        shm_unlink(SHM_NAME);
        return 1;
    }

    strncpy(data->text, "hi!", TEXT_SIZE - 1);
    data->text[TEXT_SIZE - 1] = '\0';
    printf("Server wrote: %s\n", data->text);
    sem_post(server_to_client);

    sem_wait(client_to_server);
    printf("Server got reply: %s\n", data->text);

    sem_close(server_to_client);
    sem_close(client_to_server);
    sem_unlink(SEM_SERVER_TO_CLIENT);
    sem_unlink(SEM_CLIENT_TO_SERVER);
    munmap(data, sizeof(ShmData));
    close(fd);
    shm_unlink(SHM_NAME);
    return 0;
}