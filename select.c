#include <sys/select.h>
#include <stdio.h>
#include <string.h>

int main() {
    int i;
    struct timeval timeout;
    struct fd_set master_set, working_set;
    char buf[5];

    FD_ZERO(&master_set);
    FD_SET(0, &master_set);
    timeout.tv_sec = 4;
    timeout.tv_usec = 0;
    i = 0;

    while (strcmp(buf, "0\n") != 0) {
        memcpy(&working_set, &master_set, sizeof(master_set));
        select(1, &working_set, NULL, NULL, &timeout);
        if (FD_ISSET(0, &working_set)) {
            fgets(buf, sizeof(buf), stdin);
            fputs(buf, stdout);
        }
        printf("iteration: %d\n", i++);
    }
    return 0;
}