#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>

#define LOGFILE "customer_log"
#define TABLE_SIZE 1000
#define BILLION 1000000000.0

typedef struct {
	int type;
	char time_type;
	double t;
} Table;

int cmp(const void* a, const void* b) {
	Table ta = *(Table*)a, tb = *(Table*)b;
	return ta.t - tb.t < 0 ? -1 : 1;
}

double limit_time[3] = {-1, 1.0, 0.3};
int sig_arr[3] = {SIGINT, SIGUSR1, SIGUSR2};
int serial_send[3], serial_fin[3];
int send0_cnt, finish_cnt[3];
FILE* fp_log;
Table tbl[TABLE_SIZE];
int tbl_cnt;

void sig_hand(int signo) {
	for (int i = 0; i < 3; i++) {
		if (signo == sig_arr[i]) {
			finish_cnt[i]++;
			fprintf(fp_log, "finish %d %d\n", i, ++serial_fin[i]);
		}
	}
}

int main(int argc, char* argv[]) {
	FILE* fp_data = fopen(argv[1], "r");
	fp_log = fopen(LOGFILE, "w");
	int ppid = getppid();

	int type;
	double t;
	tbl_cnt = 0;
	while (fscanf(fp_data, "%d%lf", &type, &t) != EOF) {
		tbl[tbl_cnt].type = type;
		tbl[tbl_cnt].time_type = 's';
		tbl[tbl_cnt].t = t;
		tbl_cnt++;
		if (type != 0) {
			tbl[tbl_cnt].type = type;
			tbl[tbl_cnt].time_type = 'f';
			tbl[tbl_cnt].t = t + limit_time[type];
			tbl_cnt++;
		}
	}
	qsort(tbl, tbl_cnt, sizeof(Table), cmp);

	struct sigaction act[3];
	for (int i = 0; i < 3; i++) {
		sigemptyset(&act[i].sa_mask);
		act[i].sa_flags = 0;
		act[i].sa_handler = sig_hand;
		sigaction(sig_arr[i], &act[i], NULL);
	}

	setbuf(stdout, NULL);
	struct timespec start, now;
	clock_gettime(CLOCK_REALTIME, &start);
	for (int i = 0; i < tbl_cnt; i++) {
		while (1) {
			clock_gettime(CLOCK_REALTIME, &now);
			if ((now.tv_sec + now.tv_nsec / BILLION) >=
				(start.tv_sec + start.tv_nsec / BILLION) + tbl[i].t)
				break;
		}
		int _type = tbl[i].type;
		if (tbl[i].time_type == 's') {
			fprintf(fp_log, "send %d %d\n", _type, ++serial_send[_type]);
			if (_type == 0) {
				printf("ordinary\n");
				send0_cnt++;
			}
			else
				kill(ppid, sig_arr[_type]);
		}
		else {
			if (finish_cnt[_type] == 0) {
				fprintf(fp_log, "timeout %d %d\n", _type, serial_send[_type]);
				return 0;
			}
			else
				finish_cnt[_type]--;
		}
	}

	while (finish_cnt[0] < send0_cnt)
		;

	fclose(fp_data);
	fclose(fp_log);
    return 0;
}
