#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void* thr_func() {
	for (int i = 1; i <= 10; i++) {
		printf("thread: %d\n", i);
	}
	return (void*)0;
}

int main(int argc, char* argv[]) {
	pthread_t tid;
	pthread_create(&tid, NULL, thr_func, NULL);

	for (int i = 1; i <= 10; i++) {
		printf("main: %d\n", i);
	}

	sleep(1);
    return 0;
}
