#include "thread.h"
#include <stdio.h>


void *fn(void *arg) {
   printf("\nINFO: This code is running under thread.\n");

   return NULL;
}


int main() {
  printf("%p\n", thread_l_head);
  thread_t tid;
  printf("%d\n", tid);

  thread_create(&tid, NULL, fn, NULL);
  printf("%p\n", thread_l_head);
  printf("%d\n", tid);
  printf("%d\n", thread_self());
  
  return 0;
}