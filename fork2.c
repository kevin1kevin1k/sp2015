#include <stdio.h>

int main() {
    int pid = fork();
    int status;

    if (pid == -1) {
        perror("fork failed\n");
        return -1;
    }
    else if (pid == 0) { // child
        printf("child says hello!\n");
        printf("getpid: %d\n", getpid());
        return 0;
    }
    else { // parent
        printf("parent says world!\n");
        waitpid(pid, &status, 0);
        printf("parent says world again!\n");
        printf("getpid: %d\n", getpid());

    }

    printf("pid: %d, status: %d\n", pid, status);

    //////

    int pid2 = fork();

    if (pid2 == -1) {
        perror("fork2 failed\n");
        return -1;
    }
    else if (pid2 == 0) { // child
        printf("child2 says hello!\n");
        printf("getpid: %d\n", getpid());
        return 0;
    }
    else { // parent
        printf("parent2 says world!\n");
        waitpid(pid2, &status, 0);
        printf("parent2 says world again!\n");        
        printf("getpid: %d\n", getpid());
    }

    printf("pid2: %d, status: %d\n", pid2, status);

    return 0;
}