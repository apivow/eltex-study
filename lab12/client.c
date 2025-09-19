#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
 

int main() {
    const char *fifo_path = "/tmp/myfifo";

    int fd = open(fifo_path, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    char buf[128];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    if (n == -1) {
        perror("read");
        close(fd);
        return 1;
    }

    buf[n] = '\0';
    printf("%s\n", buf);
    close(fd);
    if(unlink(fifo_path) == -1) {
        perror("unlink");
        return 1;
    }
    return 0;
}