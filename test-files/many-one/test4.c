#include "../thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h> 

void *printfun(void *arg) {
	printf("\ninside print\n");
	thread_exit(NULL);
	return NULL;
}

int main() {
	thread_t tid1, tid2, tid3;
	thread_create(&tid1, NULL, printfun, NULL);
	thread_create(&tid2, NULL, printfun, NULL);
	thread_create(&tid3, NULL, printfun, NULL);
	thread_kill(tid1, SIGSTOP);
	thread_join(tid1, NULL);
	thread_join(tid2, NULL);
	thread_join(tid3, NULL);
	
	return 0;
}
