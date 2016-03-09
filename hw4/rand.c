#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("Usage: ./rand HOW_MANY\n");
		return -1;
	}
	int num = atoi(argv[1]);
	printf("%d\n", num);
	srand(time(NULL));
	for (int i = 0; i < num; i++) {
		printf("%d\n", rand());
	}
    return 0;
}
