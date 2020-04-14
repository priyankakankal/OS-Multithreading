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

/*Thread Control Block structure */
typedef struct thread_struct {
	thread_t tid; 			/* The thread-id of the thread */
	int state; 			/* the state in which the corresponding thread will be. */
	void * (*start_func) (void *); 	/* The func pointer to the thread function to be executed. */
	void *arg; 			/* The arguments to be passed to the thread function. */
	void *returnValue; 			/* The return value that thread returns. */
	struct thread_struct *blockedForJoin; 	/* Thread blocking on this thread */
	struct thread_struct *prev, *next;	/* Pointers to next and previous TCB */
} thread_struct;


// Global pointer to the head node in the queue of Thread Structure
thread_struct *thread_l_head;

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
		perror("ERROR: Unable to allocate memory for stack.\n");
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


int thread_join(thread_t thread, void **retval) {
	thread_struct *this_thread, *waitfor_thread;
	thread_t t;
	waitfor_thread = search_thread(thread);
	this_thread = search_thread(thread_self());

	/* If the thread is already dead, no need to wait. Just collect the return
	 * value and exit
	 */
	if (waitfor_thread->state == DEAD) {
		if(retval)
			*retval = waitfor_thread->returnValue;
		return 0;
	}

	/* If the thread is not dead and someone else is already waiting on it
	 * return an error
	 */
	if (waitfor_thread->blockedForJoin != NULL)
		return -1;

	waitfor_thread->blockedForJoin = this_thread;
	this_thread->state = BLOCKED;

	t = waitpid(thread, NULL, 0);

	if(t == -1)
		perror("thread has exited\n");

	/* Target thread died, collect return value and return */
	if(retval)
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
}

int thread_lock_init(thread_lock_t *mutex, const thread_attr_lock *mutexattr) {
	mutex->mut_lock = 0;
	return 0;
}

int test(thread_lock_t *mutex) {
	if(mutex->mut_lock == 0) 
		return 0;
	else 
		return 1;
}

int thread_lock(thread_lock_t *mutex) {
	while(test(mutex) != 0);
	mutex->mut_lock = 1;
	return 0;
}
	
int thread_unlock(thread_lock_t *mutex) {
	mutex->mut_lock = 0;
	return 0;
}

int thread_kill(thread_t thread, int sig) {
	int tid;
	thread_struct *this_thread;
	this_thread = search_thread(thread);
	tid = (unsigned long)this_thread->tid;
	syscall(SYS_tkill, tid, sig);
	return errno;
}
