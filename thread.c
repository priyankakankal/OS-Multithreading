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

/* Add thread_struct to the list of TCB
*/
void addthread_l(thread_struct *node) {

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


/* Get thread_structure from thread_t
*/
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
		if(tmp->tid == tid) {
			return tmp;
		}
		tmp = tmp->next;
	}

	return NULL;

}

/* Create TCB for main thread to list of threads
*/
int addmain_thread(void) {
	thread_struct *m;
	m = (thread_struct *)malloc(sizeof(thread_struct));
	if (m == NULL ) {
		perror("ERROR: Unable to allocate memory for main thread.\n");
		return errno;
	}

	m->start_func = NULL;
	m->arg = NULL;
	m->state = READY;
	m->returnValue = NULL;
	m->blockedForJoin = NULL;
	m->tid = thread_self();

	addthread_l(m);
	return 0;
}

int newfn(void *thread) {
	thread_struct *newt;
	newt = (thread_struct *)thread;

	newt->start_func(newt->arg);

	return 0;
}

int thread_create(thread_t *t, const thread_attr_t * attr, void * (*start_function)(void *), void *arg) {
	//one-one and many-one model

	thread_t tid;
	unsigned long stack_size;
	char *tchild_stack; // Pointer to stack of child thread
	char *stackTop;
	thread_struct *child_thread;
	int ret;

	if(!thread_l_head) {
		/* first create call
			Add main thread
		*/

		ret = addmain_thread();

		if(ret != 0) {
			return ret;
		}

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
		return errno;
	}

	stackTop = tchild_stack + stack_size;

	/*
		Create thread_struct for child thread
	*/
	child_thread = (thread_struct *)malloc(sizeof(thread_struct));
	if (child_thread == NULL ) {
		perror("ERROR: Unable to allocate memory for thread_struct.\n");
		return errno;
	}

	child_thread->start_func = start_function;
	child_thread->arg = arg;
	child_thread->state = READY;
	child_thread->returnValue = NULL;
	child_thread->blockedForJoin = NULL;
	child_thread->tid = thread_l_head->prev->tid + 1;

	/* Add created thread_struct to list of thread blocks
	*/
	addthread_l(child_thread);


	tid = clone(newfn, stackTop, CLONE_VM | SIGCHLD, child_thread);


	if ( tid < 0 ) {
        printf("ERROR: Unable to create the child process.\n");
        exit(EXIT_FAILURE);
    }


	child_thread->tid = tid;
	*t = tid;
	return 0;
}

/* Return thread_t of calling thread
*/
thread_t thread_self(void) {
	thread_t self_tid;

	self_tid = (pid_t) syscall(SYS_gettid);

	return self_tid;
}

/* Schedule next ready thread
*/
void thread_sched(void) {

}

int thread_join(thread_t thread, void **retval) {
	thread_struct *this_thread, *waitfor_thread;
	thread_t t;
	waitfor_thread = search_thread(thread);
	this_thread = search_thread(thread_self());

	/* If the thread is already dead, no need to wait. Just collect the return
	 * value and exit
	 */
	if (waitfor_thread->state == DEAD) {
		*retval = waitfor_thread->returnValue;
		return 0;
	}

	/* If the thread is not dead and someone else is already waiting on it
	 * return an error
	 */
	if (waitfor_thread->blockedForJoin != NULL)
		return -1;

	waitfor_thread->blockedForJoin = this_thread;
	printf("Join: Setting state of %ld to %d\n",(unsigned long)this_thread->tid, thread);
	this_thread->state = BLOCKED;

	/* Schedule another thread, 
	*/
	//thread_sched(); 
	//if not one-one
	t = waitpid(thread, NULL, 0);

	if(t == -1)
		perror("thread has exited\n");

	/* Target thread died, collect return value and return */
	*retval = waitfor_thread->returnValue;
	return 0;
}

void thread_exit(void *retval) {
	thread_struct *this_thread;
	this_thread = search_thread(thread_self());
	this_thread->returnValue = retval;

	if(this_thread->blockedForJoin != NULL)
		this_thread->blockedForJoin->state = READY;

	this_thread->state = DEAD;

	//syscall(SYS_exit, 0);
}


