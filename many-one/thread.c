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
#include<setjmp.h>
#include "../thread.h"

/*Thread Control Block structure */
typedef struct thread_struct {
	thread_t tid; 			/* The thread-id of the thread */
	int state; 			/* the state in which the corresponding thread will be. */
	void * (*start_func) (void *); 	/* The func pointer to the thread function to be executed. */
	void *arg; 			/* The arguments to be passed to the thread function. */
	void *returnValue; 			/* The return value that thread returns. */
	struct thread_struct *blockedForJoin; 	/* Thread blocking on this thread */
	jmp_buf buf;
	//char *stackTop;
	struct thread_struct *prev, *next;
} thread_struct;

typedef struct ready_thread {
	thread_t tid;
	struct ready_thread *prev, *next;
} ready_thread;

// Global pointer to the head node in the queue of Thread Structure
thread_struct *thread_l_head;

ready_thread *readyqueue;

thread_struct *currently_running;



void addthread_to_ready(thread_t tid) {
	ready_thread *tmp;
	tmp = (ready_thread *)malloc(sizeof(ready_thread));
	tmp->tid = tid;
	if(readyqueue == NULL) {
		readyqueue = tmp;
		tmp->next = tmp->prev = tmp;
		return;
	}

	readyqueue->prev->next = tmp;
	tmp->prev = readyqueue->prev;
	tmp->next = readyqueue;
	readyqueue->prev = tmp;
	return;
}

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

/* Return thread_t of calling thread
*/
thread_t thread_self(void) {
	thread_t self_tid;

	self_tid = (pid_t) syscall(SYS_gettid);

	return self_tid;
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
	//printf("%d\n", m->tid);
	//m->stackTop = NULL;

	addthread_l(m);
	return 0;
}

thread_struct * findnext_ready(void) {
	thread_t t = currently_running->tid;
	ready_thread *tmp = readyqueue;
	if(!tmp)
		return NULL;

	do {
		if(tmp->tid == t) {
			return search_thread(tmp->next->tid);
		}
		tmp = tmp->next;
	} while(tmp != readyqueue);

	return NULL;
}

void removefrom_ready(thread_struct *thread) {
	ready_thread *tmp = readyqueue;
	if(!tmp)
		return;

	do {
		if(tmp->tid == thread->tid) {
			//remove node pointed by tmp
			if(readyqueue == tmp) {
				if(readyqueue->next == readyqueue) {
					tmp->next = tmp->prev = NULL;
					free(tmp);
					readyqueue = NULL;
					
				} else {
					readyqueue = readyqueue->next;
					readyqueue->prev = tmp->prev;
					if(readyqueue->next == tmp)
						readyqueue->next = readyqueue;
					tmp->next = tmp->prev = NULL;
					free(tmp);
				}
				return;
			} else {
				tmp->prev->next = tmp->next;
				tmp->next->prev = tmp->prev;
				tmp->next = tmp->prev = NULL;
				free(tmp);
			}
			return;
		}
		tmp = tmp->next;
	} while(tmp != readyqueue);
}

/*Alarm handler*/
void scheduler(){
	printf("\nBuzz!\n");

	thread_struct *n;
	//t = search_thread(currently_running->tid);
	//printf("%p\n", t->buf);
	
	if (setjmp(currently_running->buf)) {
		printf("Again in main\n");
  	}
    else
    {
    	n = findnext_ready();
    	currently_running = n;
    	ualarm(10, 0);
        longjmp(n->buf, 1);
    }
	
	//alarm(1);
}
int thread_create(thread_t *t, const thread_attr_t * attr, void * (*start_function)(void *), void *arg) {
	//one-one and many-one model

	//unsigned long stack_size;
	//char *tchild_stack; // Pointer to stack of child thread
	//char *stackTop;
	thread_struct *child_thread;
	int ret;
	printf("into create\n");
	//jmp_buf buf;

	if(!thread_l_head) {
		/* first create call
			Add main thread
		*/
		struct sigaction action;
		action.sa_handler = scheduler;
		action.sa_flags = SA_RESTART; //<-- restart 
		sigaction(SIGALRM, &action, NULL);

	
		ret = addmain_thread();

		addthread_to_ready(thread_l_head->tid);

		if(ret != 0) {
			return ret;
		}

		currently_running = thread_l_head;

	}


	//stackTop = tchild_stack + stack_size;

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
	//child_thread->stackTop = stackTop;
	*t = child_thread->tid;
	/* Add created thread_struct to list of thread blocks
	*/

	addthread_l(child_thread);

	addthread_to_ready(child_thread->tid);

	//scheduler();

	
	
	//pause();

	if (setjmp(currently_running->buf)) {
		printf("Again in main\n");
  	}
    else
    {
    	currently_running = child_thread;
		ualarm(10, 0);
		start_function(arg);
    }

	return 0;
}

int thread_join(thread_t thread, void **retval) {
	thread_struct *this_thread, *waitfor_thread;

	waitfor_thread = search_thread(thread);
	this_thread = currently_running;

	printf("into join %d\n", waitfor_thread->tid);

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

	removefrom_ready(this_thread);
	scheduler();

	/* Target thread died, collect return value and return */
	*retval = waitfor_thread->returnValue;

	return 0;
}

void thread_exit(void *retval) {
	thread_struct *this_thread;
	this_thread = currently_running;
	this_thread->returnValue = retval;

	printf("in exit %d\n", currently_running->tid);
	if(this_thread->blockedForJoin != NULL) {
		this_thread->blockedForJoin->state = READY;
		addthread_to_ready(this_thread->blockedForJoin->tid);
	}

	this_thread->state = DEAD;

	
	removefrom_ready(this_thread);
	currently_running = findnext_ready();

}
