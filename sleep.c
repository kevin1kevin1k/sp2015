#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
int main() {

    if (fork() == 0) {
        while (1) {
            sleep(3);
            printf("ya\n");
        }
    }
    else
        _exit(0);
    return 0;
}
