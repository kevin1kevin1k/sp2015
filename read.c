#include <stdio.h>
#include <fcntl.h>

int main() {
    int fd1, fd2;
    char c;
    fd1 = open("foobar", O_RDONLY, 0);
    fd2 = open("foobar", O_RDONLY, 0);
    int i;
    for (i = 0; i < 6; i++) {
        read(fd1, &c, 1);
        printf("c = %c\n", c);
        read(fd2, &c, 1);
        printf("c = %c\n", c);
    }
    exit(0);
}
