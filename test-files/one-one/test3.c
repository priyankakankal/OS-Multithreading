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
#include "../thread.h"

thread_lock_t lock;

thread_attr_lock mutexattr;

int a =100;
int b = 200;

void * fn1(void *arg){
  thread_lock(&lock);
  printf("\ninto thread1\n");
  thread_exit(&a);
 //thread_unlock(&lock);
  return NULL;
}

void * fn2(void *arg){
  thread_lock(&lock);
  printf("\ninto thread2\n");
  thread_exit(&b);
  
  return NULL;
}

int main() {
  thread_t tid, pid;
  int *status1 = NULL, *status2 = NULL;
  int ret;

  thread_lock_init(&lock, NULL);

  thread_create(&tid, NULL, &fn1, NULL);
  
  ret = thread_kill(tid, SIGTERM);

  thread_create(&pid, NULL, &fn2, NULL);
  printf("%d %d %d\n", thread_self(), tid, pid);

  if(ret != 0) {
    thread_join(tid, (void **) &status1);
    thread_join(pid, (void **) &status2);
    printf("%d %d\n", *status1, *status2);
  }
  else 
    printf("Error: Thread1 terminated\n");
  
  return 0;
}
