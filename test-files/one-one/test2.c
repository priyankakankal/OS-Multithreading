#include <stdio.h>
#include "../thread.h"
#include <stdlib.h>
#define NO_THREADS 3

int **matA, **matB, **matC;
int rows1, cols2, rows2, thread_no;
void* multiply();

int main() {
	thread_t threads[NO_THREADS];
	int i = 0, j = 0, x = 0;
	int r1, c1, r2, c2;
	printf("enter no of rows, cols of matrix A");
	scanf("%d%d", &r1, &c1);
	printf("enter no of rows, cols of matrix B\n");
	scanf("%d%d", &r2, &c2);
	matA = (int **)malloc(r1 * sizeof(int *));
	matB = (int **)malloc(r2 * sizeof(int *));
	matC = (int **)malloc(r1 * sizeof(int *));
	for(x = 0; x < r1; x++) {
		matA[x] = (int *)malloc(c1 * sizeof(int));
	}
	for(x = 0; x < r2; x++) {
		matB[x] = (int *)malloc(c2 * sizeof(int));
	}
	for(x = 0; x < r1; x++) {
		matC[x] = (int *)malloc(c2 * sizeof(int));
	}
	if(c1 != r2) {
		printf("multiplication not possible");
	}
	else {
		rows1 = r1;
		cols2 = c2;
		rows2 = c1;
		printf("enter matrix A");
		for(i = 0; i < r1; i++) {
			for(j = 0; j < c1; j++) {
				scanf("%d", &matA[i][j]);
			}
		}
		printf("enter matrix B");
		for(i = 0; i < r2; i++) {
			for(j = 0; j < c2; j++) {
				scanf("%d", &matB[i][j]);
			}
		}
		for(i = 0; i < NO_THREADS; i++) {
			if(i == 0) {
				thread_no = 0;
			}
			thread_create(&threads[i], NULL, &multiply, NULL);
		}
		for(i = 0; i < NO_THREADS; i++) {
			thread_join(threads[i], NULL);
		}
		printf("\n");
		for(i = 0; i < r1; i++) {
			for(j = 0; j < c2; j++) {
				printf("%d\t", matC[i][j]);
			}
			printf("\n");
		}
	}
	free(matA);
	free(matB);
	free(matC);
	return 0;
}

void* multiply() {
	int a, j, k = 0;
	for(a = (thread_no * rows1) / 3; a < (thread_no + 1) * rows1 / 3; a++) {
		for(j = 0; j < cols2; j++) {
			for(k = 0; k < rows2; k++) {
                		matC[a][j] += matA[a][k] * matB[k][j];
			}
		}
	}
	thread_no++;
	thread_exit(matC);
	return(NULL);
}






