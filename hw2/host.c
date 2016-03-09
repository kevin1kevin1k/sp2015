/* b03902086 李鈺昇 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

#define CHILD 0
#define FIFO_LEN 20
#define LINE_LEN 30

int compare(const void *a, const void *b) {
	return *(int*)b - *(int*)a; // descending
}

int find_winner(int v[4]) {
	int w[4];
	for (int i = 0; i < 4; i++)
		w[i] = v[i];
	qsort(w, 4, sizeof(int), compare); // descending

	int val;
	if (w[0] > w[1])
		val = w[0]; // 0>1...
	else if (w[1] > w[2] && w[2] > w[3])
		val = w[2]; // 0=1>2>3
	else if (w[2] > w[3])
		val = w[3]; // 0=1=2>3

	for (int i = 0; i < 4; i++)
		if (v[i] == val)
			return i;
	return -1;
}

void get_rank(int score[], int rank[], int n) {
	int w[n];
	for (int i = 0; i < n; i++)
		w[i] = score[i];
	qsort(w, n, sizeof(int), compare); // descending

	for (int i = 0, now = 1, cnt; i < n; i += cnt, now += cnt) {
		cnt = 0;
		for (int j = 0; j < n; j++) {
			if (score[j] == w[i]) {
				rank[j] = now;
				cnt++;
			}
		}
	}
}

int main(int argc, char* argv[]) {
	char* host_id = argv[1];

	mode_t mode = S_IRUSR | S_IRGRP | S_IROTH
				| S_IWUSR | S_IWGRP | S_IWOTH;

	char w_fifo[4][FIFO_LEN], r_fifo[FIFO_LEN];
	for (int i = 0; i < 4; i++) {
		sprintf(w_fifo[i], "host%s_%c.FIFO", host_id, 'A' + i);
		mkfifo(w_fifo[i], mode);
	}
	sprintf(r_fifo, "host%s.FIFO", host_id);
	mkfifo(r_fifo, mode);

	int player_index[4], rand_int[4];
	while (1) {
		for (int i = 0; i < 4; i++)
			scanf("%d",&player_index[i]);
		if (player_index[0] == -1)
			break;

		srand(time(NULL));
		for (int i = 0; i < 4; i++)
			rand_int[i] = rand() % 65536;

		for (int i = 0; i < 4; i++) {
			if (fork() == CHILD) {
				char random_key[7], p_idx[3];
				sprintf(random_key, "%d", rand_int[i]);
				sprintf(p_idx, "%c", 'A' + i);
				execl("./player", "./player", host_id, p_idx, random_key, (char *)0);
			}
		}

		int w_fd[4], r_fd;
		FILE *w_fp[4], *r_fp;
		for (int i = 0; i < 4; i++) {
			w_fd[i] = open(w_fifo[i], O_WRONLY);
			w_fp[i] = fdopen(w_fd[i], "w");
		}
		r_fd = open(r_fifo, O_RDONLY);
		r_fp = fdopen(r_fd, "r");

		int money[4] = {0}, score[4] = {0}, pay[4];
		char line[LINE_LEN];
		for (int i = 0; i < 10; i++) {
			int cnt = 0;
			for (int j = 0; j < 4; j++) {
				money[j] += 1000;
				cnt += sprintf(line + cnt, "%d ", money[j]);
			}
			sprintf(line + cnt, "\n");

			for (int j = 0; j < 4; j++) {
				fputs(line, w_fp[j]);
				fflush(w_fp[j]);
			}

			for (int j = 0; j < 4; j++) {
				while (fgets(line, LINE_LEN, r_fp) == NULL)
					;
				char p_i;
				int p;
				sscanf(line, "%c%*d%d", &p_i, &p);
				pay[p_i - 'A'] = p;
			}

			int winner = find_winner(pay);
			if (winner >= 0) {
				score[winner]++;
				money[winner] -= pay[winner];
			}
		}

		int rank[4];
		get_rank(score, rank, 4);
		char idx_rank[40];
		int cnt = 0;
		for (int i = 0; i < 4; i++)
			cnt += sprintf(idx_rank + cnt, "%d %d\n", player_index[i], rank[i]);
		fputs(idx_rank, stdout);
		fflush(stdout);

		for (int i = 0; i < 4; i++)
			wait(NULL);

		for (int i = 0; i < 4; i++)
			fclose(w_fp[i]);
		fclose(r_fp);

	}
	for (int i = 0; i < 4; i++)
		unlink(w_fifo[i]);
	unlink(r_fifo);

    return 0;
}
