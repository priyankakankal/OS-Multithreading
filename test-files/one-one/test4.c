#include "../thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h> 
#include <unistd.h>

int sum[16];


void *add(void *arg) {
  int *t = (int *)arg, j, index;
  index = t[2];
  sum[index] = 0;
  for(j = t[0]; j <= t[1]; j++) {
    sum[index] += j;
  }
  thread_exit(&(sum[index]));	
  return NULL;
}

int main(int argc, char *argv[]) {
  thread_t tid[16];
  int *res[16], sum = 0;
  int i, n ,m;
  int d[16][3];

  n = atoi(argv[1]);
  m = atoi(argv[2]);

  for(i = 0; i < m; i++) {
    d[i][0] = i * (n/m) + 1;
    d[i][1] = (i + 1) * (n/m);
    d[i][2] = i;
    thread_create(&tid[i], NULL, add, d[i]);
  }
if(thread_kill(tid[0], SIGKILL) == 0)
	printf("Error: Thread1 killed\n");
  
else{
  for(i = 0; i < m; i++) {
    thread_join(tid[i], (void **) &(res[i]));
    sum += *(res[i]);
  }
  printf("%d\n", sum);
}
  return 0;
}
