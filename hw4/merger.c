#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_N 50000000

typedef struct {
	int *arr;
	int num;
} Arr_Num;

int n, v[MAX_N], step;
Arr_Num *an_list[MAX_N];
pthread_t tid[MAX_N], tid2[MAX_N];
pthread_mutex_t mutex;

int cmp(const void *a, const void *b) {
	return *(int*)a - *(int*)b;
}

int min(int a, int b) {
	return a < b ? a : b;
}

int _ceil(int a, int b) {
	if (a % b == 0) {
		return a / b;
	}
	else {
		return a / b + 1;
	}
}

void *sort(void *arg) {

	pthread_mutex_lock(&mutex);
	long idx = (long)arg;
	printf("Handling elements:\n");
	int *_arr = an_list[idx]->arr, _num = an_list[idx]->num;
	for (int i = 0; i < _num; i++) {
		printf("%d%s", _arr[i], i+1 == _num ? "\n" : " ");
	}
	printf("Sorted %d elements.\n", _num);
	pthread_mutex_unlock(&mutex);

	qsort(_arr, _num, sizeof(int), cmp);
	return (void*)0;
}

void *merge(void *arg) {

	long L = (long)arg, R = L + step/2;
	int nl = an_list[L]->num, nr = an_list[R]->num, total = nl + nr;
	int dup = 0;

	int i = 0, il = 0, ir = 0;
	int *tmp = (int*)malloc(sizeof(int)*total);
	while (i < total && il < nl && ir < nr) {
		int d = an_list[L]->arr[il] - an_list[R]->arr[ir];
		if (d <= 0) {
			tmp[i++] = an_list[L]->arr[il++];
			if (d == 0) {
				dup++;
			}
		}
		else if (d > 0) {
			tmp[i++] = an_list[R]->arr[ir++];
		}
	}

	while (il < nl) {
		tmp[i++] = an_list[L]->arr[il++];
	}
	while (ir < nr) {
		tmp[i++] = an_list[R]->arr[ir++];
	}


	pthread_mutex_lock(&mutex);
	printf("Handling elements:\n");
	for (int i = 0; i < total; i++) {
		printf("%d%s", an_list[L]->arr[i], i+1 == total ? "\n" : " ");
	}
	printf("Merged %d and %d elements with %d duplicates.\n", nl, nr, dup);
	pthread_mutex_unlock(&mutex);

	an_list[L]->num = total;
	for (int i = 0; i < total; i++) {
		an_list[L]->arr[i] = tmp[i];
	}

	free(tmp);
	return (void*)0;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: ./merger [segment_size]\n");
		return -1;
	}
	int seg_size = atoi(argv[1]);
	scanf("%d", &n);
	for (int i = 0; i < n; i++) {
		scanf("%d", &v[i]);
	}
	int thr_num = _ceil(n, seg_size);
	for (int i = 0; i < thr_num; i++) {
		an_list[i] = (Arr_Num*)malloc(sizeof(Arr_Num));
		an_list[i]->arr = v + i*seg_size;
		an_list[i]->num = min(seg_size, n - i*seg_size);
	}

	pthread_mutex_init(&mutex, NULL);
	for (long i = 0; i < thr_num; i++) {
		pthread_create(&tid[i], NULL, sort, (void*)i);
	}
	for (long i = 0; i < thr_num; i++) {
		pthread_join(tid[i], NULL);
	}

	step = 2;
	int cnt = thr_num;
	while (cnt > 1) {
		int round = cnt / 2;
		for (long i = 0; i < round; i++) {
			pthread_create(&tid2[i], NULL, merge, (void*)(i * step));
		}
		for (long i = 0; i < round; i++) {
			pthread_join(tid2[i], NULL);
		}
		cnt -= round;
		step *= 2;
	}

	for (int i = 0; i < n; i++) {
		printf("%d%s", v[i], i+1 == n ? "\n" : " ");
	}
	printf("\n");

	for (int i = 0; i < thr_num; i++) {
		free(an_list[i]);
	}
	pthread_mutex_destroy(&mutex);
	return 0;
}
