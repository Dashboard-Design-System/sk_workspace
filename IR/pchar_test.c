#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#define DEVFILE "/dev/pchar"
#define BUFSIZE 32

int main() {
    int fd, ret;
    char wbuf[BUFSIZE] = "Hello DESD!\n";
    char rbuf[BUFSIZE] = {0};

    // Open device for writing
    fd = open(DEVFILE, O_WRONLY);
    if (fd < 0) {
        perror("open() failed (write)");
        exit(EXIT_FAILURE);
    }
    printf("open() for write: fd=%d\n", fd);

    ret = write(fd, wbuf, strlen(wbuf));
    printf("write() returned: %d\n", ret);

    close(fd);
    printf("Device closed after write.\n");

    // Open device for reading
    fd = open(DEVFILE, O_RDONLY);
    if (fd < 0) {
        perror("open() failed (read)");
        exit(EXIT_FAILURE);
    }
    printf("open() for read: fd=%d\n", fd);

    ret = read(fd, rbuf, BUFSIZE-1);
    if (ret < 0) {
        perror("read() failed");
        close(fd);
        exit(EXIT_FAILURE);
    }
    rbuf[ret] = '\0';
    printf("read() returned: %d, read content: %s\n", ret, rbuf);

    close(fd);
    printf("Device closed after read.\n");

    return 0;
}
