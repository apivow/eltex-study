#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int main() {
    const char *fifo_path = "/tmp/myfifo";

    if(mkfifo(fifo_path, 0666) == -1) {
        if (errno != EEXIST) {
            perror("mkfifo");
            return 1;
        }
    }

    int fd = open(fifo_path, O_WRONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    const char *msg = "Hi!";
    ssize_t n = write(fd, msg, strlen(msg));
    if (n == -1) {
        perror("write");
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}