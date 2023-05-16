#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

#define BUFFER_SIZE 13

#define DEVICE_NAME "/dev/opdracht4-0"

int main(){
    int fd, ret;
    char buffer[BUFFER_SIZE];
    memset(buffer, 'z', BUFFER_SIZE);

    fd = open(DEVICE_NAME, O_RDWR);
    if (!fd) {
        printf("error, could not open " DEVICE_NAME ".\n");
    } else {
        printf("successfully opened " DEVICE_NAME ".\n");
    }
    printf("Writing...");
    ret = write(fd, "BB\n", 3);
    printf("ret = %d.\n", ret);

    lseek(fd, 6, SEEK_SET);
    ret = write(fd, "DD", 2);
    printf("ret = %d.\n", ret);

    lseek(fd, 0, SEEK_SET);
    ret = read(fd, buffer, BUFFER_SIZE);
    printf("ret = %d.\n", ret);

    buffer[BUFFER_SIZE] = '\0';
    for (int i = i; i < BUFFER_SIZE; i++) {
        printf("%d: %d.\n", i, (int)buffer[i]);
    }
    // printf("Read: %s\n", buffer);

    close(fd);

    return 0;
}
