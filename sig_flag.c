#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int flag;

void sig_int(int signo) {
	signal(SIGINT, sig_int);
	flag = 1;
}

int main(int argc, char* argv[]) {
	signal(SIGINT, sig_int);
	flag = 0;

	for (int i = 0; i < 10; i++) {
		sleep(1);
		printf("sleep %d done\n", i);
	}

	while (flag == 0) {
		pause();
	}

	printf("ok, got the signal\n");
    return 0;
}
