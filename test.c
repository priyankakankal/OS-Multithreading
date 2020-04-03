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
#include "thread.h"

void * fn(void *arg){
  int i = 100;
  //printf("into thread\n");
  
  printf("%d Calling thread_exit\n", thread_self());
  thread_exit(&i);
  return NULL;
}


int main() {
  thread_t tid, pid;
  int *status1 = NULL, *status2 = NULL;

  printf("in test1\n");
  
  thread_create(&tid, NULL, &fn, NULL);
  thread_create(&pid, NULL, &fn, NULL);

  thread_join(tid, (void **) &status1);
  thread_join(pid, (void **) &status2);

  printf("%d %d\n", *status1, *status2);
  return 0;
}
