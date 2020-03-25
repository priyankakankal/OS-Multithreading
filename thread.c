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


void addthread_l(thread_struct *node) {
	thread_struct *tmp;

	if(thread_l_head == NULL) {
		thread_l_head = node;
		node->next = node->prev = node;
		return;
	}


	thread_l_head->prev->next = node;
	node->prev = thread_l_head->prev;
	node->next = thread_l_head;
	thread_l_head->prev = node;

	return;
}


// Get thread_structure from thread_t
thread_struct * search_thread(thread_t tid) {
	thread_struct *tmp;

	if(!thread_l_head) {
		perror("No threads created");
		return NULL;
	}

	if(thread_l_head->tid == tid)
		return thread_l_head;

	tmp = thread_l_head->next;

	while(tmp != thread_l_head) {
		if(tmp->tid == tid)
			return tmp;
		tmp = tmp->next;
	}

	return NULL;

}


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
	*t = tid;
	
	return 0;
}

int thread_join(thread_t thread, void **retval) {
	thread_struct *this_thread, *waitfor_thread;

	waitfor_thread = search_thread(thread);
}

thread_t thread_self(void) {
	thread_t self_tid;

	self_tid = (pid_t) syscall(SYS_gettid);

	return self_tid;
}
