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


int thread_create(thread_t *t, const thread_attr_t * attr, void * (*start_function)(void *), void *arg) {
	//one-one and many-one model

	thread_t tid;
	unsigned long stack_size;
	char *tchild_stack; // Pointer to stack of child thread
	char *stackTop;
	thread_struct *child_thread;

	if(!thread_l_head) {
		/* first create call
			Initialize linked list of thread control block
		*/
	}


	/* Create stack for child thread
	*/
	if (attr == NULL)
		stack_size = SIGSTKSZ;
	else
		stack_size = attr->stack_size;

	tchild_stack = malloc(stack_size);
	if ( tchild_stack == NULL ) {
		printf("ERROR: Unable to allocate memory for stack.\n");
		exit(EXIT_FAILURE);
	}

	stackTop = tchild_stack + stack_size;

	/*
		Create thread_struct for child thread
	*/
	child_thread = (thread_struct *)malloc(sizeof(thread_struct));
	if (child_thread == NULL ) {
		perror("ERROR: Unable to allocate memory for thread_struct.\n");
		exit(EXIT_FAILURE);
	}

	child_thread->start_func = start_function;
	child_thread->arg = arg;
	child_thread->state = READY;
	child_thread->returnValue = NULL;
	child_thread->blockedForJoin = NULL;

	/* Add created thread_struct to list of thread blocks
	*/
	addthread_l(child_thread);

	tid = clone(start_function, stackTop, SIGCHLD, NULL);

	if ( tid < 0 ) {
        printf("ERROR: Unable to create the child process.\n");
        exit(EXIT_FAILURE);
    }


	child_thread->tid = tid;
	
	return 0;
}

int thread_join(thread_t thread, void **retval) {
}


/*for testing only*/
void *fn(void *arg) {
   printf("\nINFO: This code is running under child process.\n");

   return NULL;
}

int main() {
	printf("%p\n", thread_l_head);
	thread_t tid;

	thread_create(&tid, NULL, fn, NULL);
	printf("%p\n", thread_l_head);
	
	return 0;
}
