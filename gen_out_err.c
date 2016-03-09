#include <stdio.h>

int main() {
    char s[100];
    scanf("%s", s);
    printf("This is stdout, %s!\n", s);
    fflush(stdout);
    fprintf(stderr, "This is stderr, %s!\n", s);

    return 0;
}
