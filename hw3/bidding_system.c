#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>

#define LOGFILE "bidding_system_log"
#define BUF_SIZE 20
#define BILLION 1000000000

double proc_time[3] = {1.0, 0.5, 0.2};
int sig_arr[3] = {SIGINT, SIGUSR1, SIGUSR2};
int serial_rec[3], serial_fin[3];
FILE* fp_log;
int pid;

void sig_hand(int signo) {
	for (int i = 0; i < 3; i++) {
		if (signo == sig_arr[i]) {
			fprintf(fp_log, "receive %d %d\n", i, ++serial_rec[i]);
			struct timespec req, rem;
			double d = proc_time[i];
			req.tv_sec = (int)d;
			req.tv_nsec = (d - (int)d) * BILLION;
			while (nanosleep(&req, &rem) == -1)
			req = rem;
			kill(pid, sig_arr[i]);
			fprintf(fp_log, "finish %d %d\n", i, ++serial_fin[i]);
		}
	}
}

int main(int argc, char* argv[]) {

	struct sigaction act_mem, act_vip;
	sigemptyset(&act_mem.sa_mask);
	sigemptyset(&act_vip.sa_mask);
	sigaddset(&act_vip.sa_mask, SIGUSR1);
	act_mem.sa_flags = 0;
	act_vip.sa_flags = 0;
	act_mem.sa_handler = sig_hand;
	act_vip.sa_handler = sig_hand;
	sigaction(SIGUSR1, &act_mem, NULL);
	sigaction(SIGUSR2, &act_vip, NULL);

	int pfd[2];
	pipe(pfd);
	pid = fork();
	if (pid == 0) {
		close(pfd[0]);
		dup2(pfd[1], STDOUT_FILENO);
		close(pfd[1]);
		execl("./customer", "./customer", argv[1], (char*)0);
	}
	close(pfd[1]);

	fp_log = fopen(LOGFILE, "w");
	FILE* fp_pipe = fdopen(pfd[0], "r");
	char buf[BUF_SIZE];

	while (1) {
		if (fgets(buf, BUF_SIZE, fp_pipe) == NULL) { // EOF or signal
			if (feof(fp_pipe)) {
				break;
			}
		}
		else
			sig_hand(SIGINT);
	}

	wait(NULL);
	fprintf(fp_log, "terminate\n");
	fclose(fp_pipe);
	fclose(fp_log);
    return 0;
}
