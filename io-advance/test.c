
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    ssize_t num = -1;
    int fd = 1;
    char buf[12] = {'\0'};

    fd = open("./test.txt", O_RDONLY);
    if (fd == -1) {
        printf("open() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    num = read(fd,buf,10);
}
