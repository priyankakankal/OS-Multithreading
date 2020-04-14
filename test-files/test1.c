#include "../thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h> 

int sum[16];
thread_lock_t lock;

thread_attr_lock mutexattr;

void *add(void *arg) {
  int *t = (int *)arg, j, index;
  thread_lock(&lock);
  index = t[2];
  sum[index] = 0;
  for(j = t[0]; j <= t[1]; j++) {
    sum[index] += j;
  }
  thread_unlock(&lock);
  thread_exit(&(sum[index]));
  return NULL;
}

int main(int argc, char *argv[]) {
  thread_t tid[16];
  int *res[16], sum = 0;
  int i, n ,m;
  int d[16][3];
  thread_lock_init(&lock, NULL);

  n = atoi(argv[1]);
  m = atoi(argv[2]);

  for(i = 0; i < m; i++) {
    d[i][0] = i * (n/m) + 1;
    d[i][1] = (i + 1) * (n/m);
    d[i][2] = i;
    thread_create(&tid[i], NULL, add, d[i]);
  }
  
  thread_kill(tid[0], SIGSTOP);
  printf("%s\n", strerror(errno));
  
  thread_kill(tid[0], SIGCONT);
  
  for(i = 0; i < m; i++) {
    thread_join(tid[i], (void **) &(res[i]));
    sum += *(res[i]);
  }
  printf("%d\n", sum);
  return 0;
}
