/* b03902086 李鈺昇 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define FIFO_LEN 20
#define LINE_LEN 30

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

int main(int argc, char* argv[]) {
	char* host_id = argv[1];
	char player_index = argv[2][0];
	char* random_key = argv[3];

	char r_fifo[FIFO_LEN], w_fifo[FIFO_LEN];
	sprintf(r_fifo, "host%s_%c.FIFO", host_id, player_index);
	int r_fd = open(r_fifo, O_RDONLY);
	sprintf(w_fifo, "host%s.FIFO", host_id);
	int w_fd = open(w_fifo, O_WRONLY);

	char line[LINE_LEN];
	int player_int = player_index - 'A';
	int money[4], pay;
	for (int i = 0; i < 10; i++) {
		while (read(r_fd, line, LINE_LEN) <= 0)
			;

		int other_max = -1;
		for (int j = 0; j < 4; j++) {
			sscanf(line, "%d", &money[j]);
			if (j != player_int)
				other_max = max(other_max, money[j]);
		}

		pay = ( (money[player_int] <= other_max) ? 999 :
			    min(money[player_int], other_max + 1)
			  );
		sprintf(line, "%c %s %d\n", player_index, random_key, pay);
		write(w_fd, line, strlen(line));
	}

	close(w_fd);
	close(r_fd);

	return 0;
}
