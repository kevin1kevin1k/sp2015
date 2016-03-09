#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>

#define SIGUSR3 SIGWINCH
#define LOGFILE "bidding_system_log"
#define BILLION 1000000000.0

typedef struct {
	int type;
	struct timespec remain;
	double deadline;
	int printed;
} Customer;

int cmp(const void* a, const void* b) {
	Customer ca = *(Customer*)a, cb = *(Customer*)b;
	return ca.deadline < cb.deadline ? -1 : 1;
}

double proc_time[3] = {0.5, 1.0, 0.2};
double limit_time[3] = {2.0, 3.0, 0.3};
int sig_arr[3] = {SIGUSR1, SIGUSR2, SIGUSR3};
int serial_rec[3], serial_fin[3];
FILE* fp_log;
int pid;
Customer tbl[3];
int tbl_cnt;
struct timespec start;
int restart;

void sig_hand(int signo) {
	for (int i = 0; i < 3; i++) {
		if (signo == sig_arr[i]) {
			struct timespec now;
			clock_gettime(CLOCK_REALTIME, &now);

			Customer* cus = &tbl[tbl_cnt];
			cus->type = i;
			double d = proc_time[i];
			cus->remain.tv_sec = (int)d;
			cus->remain.tv_nsec = (d - (int)d) * BILLION;
			cus->deadline = (now.tv_sec - start.tv_sec)
						  + (now.tv_nsec - start.tv_nsec) / BILLION
						  + limit_time[i];
			cus->printed = 0;

			tbl_cnt++;
			qsort(tbl, tbl_cnt, sizeof(Customer), cmp);
			restart = 1;
		}
	}
}

int main(int argc, char* argv[]) {

	struct sigaction act[3];
	for (int i = 0; i < 3; i++) {
		act[i].sa_handler = sig_hand;
		act[i].sa_flags = 0;
		sigemptyset(&act[i].sa_mask);
		for (int j = 0; j < 3; j++)
			if (i != j)
				sigaddset(&act[i].sa_mask, sig_arr[j]);
		sigaction(sig_arr[i], &act[i], NULL);
	}

	clock_gettime(CLOCK_REALTIME, &start);

	int pfd[2];
	pipe(pfd);
	pid = fork();
	if (pid == 0) {
		close(pfd[0]);
		dup2(pfd[1], STDOUT_FILENO);
		close(pfd[1]);
		execl("./customer_EDF", "./customer_EDF", argv[1], (char*)0);
	}
	close(pfd[1]);

	fp_log = fopen(LOGFILE, "w");

	fd_set master, working;
	FD_ZERO(&master);
	FD_SET(pfd[0], &master);
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	int nfds = pfd[0] + 1;

	while (1) {
		memcpy(&working, &master, sizeof(master));
		int sel = select(nfds, &working, NULL, NULL, &timeout);
		if (sel > 0)
			break;

		restart_flag:
		while (tbl_cnt > 0) {
			Customer* cus = &tbl[0];
			int i = cus->type;
			if (cus->printed == 0) {
				fprintf(fp_log, "receive %d %d\n", i, ++serial_rec[i]);
				cus->printed = 1;
			}

			struct timespec req = cus->remain;

			if (restart == 1) {
				restart = 0;
				goto restart_flag;
			}

			while (nanosleep(&req, &cus->remain) == -1) {
				if (restart == 1) {
					restart = 0;
					goto restart_flag;
				}
				req = cus->remain;
			}

			for (int j = 0; j + 1 < tbl_cnt; j++) {
				tbl[j] = tbl[j+1];
			}
			tbl_cnt--;
			kill(pid, sig_arr[i]);
			fprintf(fp_log, "finish %d %d\n", i, ++serial_fin[i]);
		}
	}

	wait(NULL);
	fprintf(fp_log, "terminate\n");
	close(pfd[0]);
	fclose(fp_log);
    return 0;
}
