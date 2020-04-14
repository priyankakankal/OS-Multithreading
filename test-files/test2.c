#include "../thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h> 

thread_lock_t lock;

thread_attr_lock mutexattr;

void *printfun(void *arg) {
	printf("inside print function\n");
	int ret = thread_lock(&lock);
	if(ret == 0) {
		printf("spin-locked\n");
	}
	thread_exit(NULL);
	return NULL;
}

int main() {
	thread_t tid1, tid2;
	if (thread_lock_init(&lock, NULL) != 0) { 
    	    printf("\n mutex init has failed\n"); 
    	    return 1; 
  	}
	thread_create(&tid1, NULL, printfun, NULL);
	thread_create(&tid2, NULL, printfun, NULL);
	thread_join(tid1, NULL);
	thread_join(tid2, NULL);
	return 0;
}
	
