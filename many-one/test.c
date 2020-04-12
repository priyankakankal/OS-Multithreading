#define _GNU_SOURCE
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sched.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <signal.h>
#include <errno.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ucontext.h>
#include "../thread.h"

void * fn1(void *arg){
  int i = 100;

  printf("into thread1\n");

  thread_exit(&i);
  
  return NULL;
}

void * fn2(void *arg){
  int i = 200;

  printf("into thread2\n");

  thread_exit(&i);
  
  return NULL;
}

int main() {
  thread_t tid, pid;
  int *status1 = NULL, *status2 = NULL;

  thread_create(&tid, NULL, &fn1, NULL);
  thread_create(&pid, NULL, &fn2, NULL);
  printf("%d %d %d\n", thread_self(), tid, pid);

  //thread_join(tid, (void **) &status1);
  //thread_join(pid, (void **) &status2);

  //printf("%d %d\n", *status1, *status2);
  return 0;
}
