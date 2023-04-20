#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE 13

int main(){
    int fd;
    char buffer[BUFFER_SIZE] = "Hello worl2!";

    printf("Writing: %s\n", buffer);

    fd = open("/dev/opdracht3-0", O_RDWR);
    write(fd, buffer, BUFFER_SIZE);
    close(fd);

    return 0;
}
