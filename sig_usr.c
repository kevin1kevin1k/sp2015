#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void sig_usr(int signo) {
	static int cnt = 0;

	if (signo == SIGUSR1) {
		printf("received SIGUSR1\n");
	}
	else if (signo == SIGUSR2) {
		printf("received SIGUSR2\n");
	}
	else if (signo == SIGINT) {
		printf("received SIGINT %d\n", ++cnt);
	}
	else {
		printf("received %d\n", signo);
	}
}

int main(int argc, char* argv[]) {
	signal(SIGUSR1, sig_usr);
	signal(SIGUSR2, sig_usr);
	signal(SIGINT, sig_usr);
	while (1)
		pause();
    return 0;
}
