#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv, char **envp) {

    if (argc < 3) {
        printf("3 arguments required, %d supplied.\n", argc);
        return 0;
    }

    int fd1, fd2;

    fd1 = open(argv[1], O_RDONLY);
    dup2(fd1, 0);
    close(fd1);

    fd2 = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0644);
    dup2(fd2, 1);
    close(fd2);

    char *newargv[2] = {"cat", (char*) 0};
    execve("/bin/cat", newargv, envp);

    return 0;
}
