#include <stdio.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    int fd1, fd2;

    char *infile = "input", *outfile = "output";

    fd1 = open(infile, O_RDONLY);
    fd2 = open(outfile, O_WRONLY | O_CREAT, 0666);

    dup2(fd1, 0);
    dup2(1, 2);
    dup2(fd2, 1);

    execlp("./a.out", "./a.out", (char *)0);

    return 0;
}