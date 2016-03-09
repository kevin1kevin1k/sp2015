/* b03902086 李鈺昇 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/select.h>

#define CHILD 0
#define HOST_MAX 12
#define PLAYER_MAX 20
#define COMP_MAX 4845 // C(20, 4)
#define LINE_LEN 20

int Cn4(int n) {
	return n * (n-1) * (n-2) * (n-3) / 24;
}

void get_comps(int four_i, int four[], int players_i, int players[], int max, int table[4845][4]) {
	static int cnt;
	if (four_i == 4) {
		for (int i = 0; i < 4; i++)
			table[cnt][i] = four[i];
		cnt++;
		return;
	}
	if (players_i == max)
		return;

	get_comps(four_i, four, players_i + 1, players, max, table);
	four[four_i] = players[players_i];
	get_comps(four_i + 1, four, players_i + 1, players, max, table);
}

int compare(const void *a, const void *b) {
	return *(int*)b - *(int*)a; // descending
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
	int host_num = atoi(argv[1]);
	int player_num = atoi(argv[2]);

	int pfd_bh[HOST_MAX + 1][2];
	int pfd_hb[HOST_MAX + 1][2];
	int pids[HOST_MAX + 1];
	int fd_max = -1;
	for (int i = 1; i <= host_num; i++) {
		pipe(pfd_bh[i]);
		pipe(pfd_hb[i]);
		int pid = fork();
		if (pid == CHILD) {
			close(pfd_hb[i][0]);
			close(pfd_bh[i][1]);

			dup2(pfd_bh[i][0], STDIN_FILENO);
			close(pfd_bh[i][0]);
			dup2(pfd_hb[i][1], STDOUT_FILENO);
			close(pfd_hb[i][1]);

			char host_id[5];
			sprintf(host_id, "%d", i);
			execl("./host", "./host", host_id, (char *)0);
		}
		else {
			close(pfd_bh[i][0]);
			close(pfd_hb[i][1]);

			if (pfd_hb[i][0] > fd_max)
				fd_max = pfd_hb[i][0];
			pids[i] = pid;
		}
	}

	int four[4], players[PLAYER_MAX], table[COMP_MAX][4];
	for (int i = 0; i < player_num; i++)
		players[i] = i + 1;
	get_comps(0, four, 0, players, player_num, table);

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	fd_set master_set, working_set;
	FD_ZERO(&master_set);

	int score[PLAYER_MAX + 1] = {0};
	for (int result = 0, send = 0; result < Cn4(player_num); ) {
		for (int j = 1; j <= host_num && send < Cn4(player_num); j++) {
			int fd = pfd_hb[j][0];
			if (!FD_ISSET(fd, &master_set)) {
				char line[LINE_LEN];
				int cnt = 0;
				for (int k = 0; k < 4; k++)
					cnt += sprintf(line + cnt, "%d ", table[send][k]);
				sprintf(line + cnt, "\n");
				write(pfd_bh[j][1], line, strlen(line));

				FD_SET(fd, &master_set);
				send++;
			}
		}

		int nfds = -1;
		for (int j = 1; j <= host_num; j++) {
			int fd = pfd_hb[j][0];
			if (FD_ISSET(fd, &master_set) && fd > nfds)
				nfds = fd;
		}
		nfds++;

		memcpy(&working_set, &master_set, sizeof(master_set));
		int sel = select(nfds, &working_set, NULL, NULL, &timeout);
		if (sel < 0) {
			printf("Error in select!\n");
			break;
		}
		else if (sel > 0) {
			for (int j = 1; j <= host_num; j++) {
				int fd = pfd_hb[j][0];
				if (FD_ISSET(fd, &working_set)) {
					char four_line[LINE_LEN * 4];
					read(fd, four_line, sizeof(four_line));

					int id[4], rank[4];
					sscanf(four_line, "%d%d%d%d%d%d%d%d", &id[0], &rank[0], &id[1], &rank[1], &id[2], &rank[2], &id[3], &rank[3]);
					for (int k = 0; k < 4; k++)
						score[id[k]] += (4 - rank[k]); // [1, 2, 3, 4] -> [3, 2, 1, 0]

					FD_CLR(fd, &master_set);
					result++;
				}
			}
		}
	}

	int rank[PLAYER_MAX + 1];
	get_rank(score + 1, rank + 1, player_num);

	for (int i = 1; i <= player_num; i++)
		printf("%d %d\n", i, rank[i]);

	char line[] = "-1 -1 -1 -1\n";
	for (int i = 1; i <= host_num; i++) {
		write(pfd_bh[i][1], line, strlen(line));

		close(pfd_bh[i][1]);
		close(pfd_hb[i][0]);
		waitpid(pids[i], NULL, 0);
	}

    return 0;
}
