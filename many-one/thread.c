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
#include <sys/time.h>
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
	jmp_buf buf;			/* To save the context of thread */
	struct thread_struct *prev, *next;	/* Pointers to next and previous TCB */
} thread_struct;

typedef struct ready_thread {
	thread_t tid;
	struct ready_thread *prev, *next;
} ready_thread;

// Global pointer to the head node in the queue of Thread Structure
thread_struct *thread_l_head;

ready_thread *readyqueue;

thread_struct *currently_running;

int flag = 0;

struct sigaction action;
struct itimerval timer;

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
	addthread_l(m);
	return 0;
}

thread_struct * findnext_ready(void) {
	thread_t t = currently_running->tid;
	ready_thread *tmp = readyqueue;
	if(!tmp) {
		return NULL;
	}

	do {
		if(tmp->tid == t) {
			return search_thread(tmp->next->tid);
		}
		tmp = tmp->next;
	} while(tmp != readyqueue);

	return search_thread(readyqueue->tid);
}

void removefrom_ready(thread_struct *thread) {
	ready_thread *tmp = readyqueue;
	if(!tmp)
		return;

	do {
		if(tmp->tid == thread->tid) {
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
	thread_struct *n;
	
	if (setjmp(currently_running->buf)) {
  	} else {
		n = findnext_ready();
		currently_running = n;
		longjmp(n->buf, 1);
	}

}
int thread_create(thread_t *t, const thread_attr_t * attr, void * (*start_function)(void *), void *arg) {
	thread_struct *child_thread;
	int ret;

	if(!thread_l_head) {
		/* first create call
			Add main thread
		*/

		memset(&action, 0, sizeof(action));

		action.sa_handler = scheduler;

		sigaction(SIGVTALRM, &action, NULL);

	
		ret = addmain_thread();

		addthread_to_ready(thread_l_head->tid);

		if(ret != 0) {
			return ret;
		}

		currently_running = thread_l_head;
	}


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

	*t = child_thread->tid;
	/* Add created thread_struct to list of thread blocks
	*/

	addthread_l(child_thread);

	addthread_to_ready(child_thread->tid);


	if (setjmp(currently_running->buf)) {
  	} else {
		currently_running = child_thread;
		if(!flag) {

			/* Configure the timer to expire after 10 usec... */
			timer.it_value.tv_sec = 0;
			timer.it_value.tv_usec = 10;
			/* ... and every 10 usec after that. */
			timer.it_interval.tv_sec = 0;
			timer.it_interval.tv_usec = 10;
			
			setitimer (ITIMER_VIRTUAL, &timer, NULL);

			flag = 1;
		}

		start_function(arg);
	}

	return 0;
}

int thread_join(thread_t thread, void **retval) {
	thread_struct *this_thread, *waitfor_thread;
	waitfor_thread = search_thread(thread);
	this_thread = currently_running;



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


	while(waitfor_thread->state != DEAD);

	/* Target thread died, collect return value and return */
	if(retval)
		*retval = waitfor_thread->returnValue;
		
	return 0;
}

void thread_exit(void *retval) {
	thread_struct *this_thread;

	
	this_thread = currently_running;

	if(this_thread->state == DEAD) {
		return;
	}

	this_thread->returnValue = retval;


	if(this_thread->blockedForJoin != NULL) {
		this_thread->blockedForJoin->state = READY;
		//addthread_to_ready(this_thread->blockedForJoin->tid);
	}

	this_thread->state = DEAD;

	currently_running = findnext_ready();
	removefrom_ready(this_thread);
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

int thread_kill(thread_t tid, int sig) {
	thread_struct *this_thread;
	this_thread = search_thread(tid);
	if(this_thread == NULL) //tid not found
		return ESRCH;

	switch(sig) {
		case 0: //Checks whether thread with given tid exists
				if(this_thread != NULL)
					return 0;
				return -1;

		case SIGSTOP:	kill(thread_l_head->tid, SIGSTOP);

		case SIGCONT:	kill(thread_l_head->tid, SIGCONT);

		case SIGTERM:	//SIGTERM will cause thread to exit "cleanly"
						if(this_thread->state == DEAD) {
							//Terminated already
							return 0;
						}
						this_thread->returnValue = NULL;
						this_thread->state = DEAD;
						removefrom_ready(this_thread);
						return 0;

		case SIGHUP:	kill(thread_l_head->tid, SIGHUP);

		case SIGINT:	kill(thread_l_head->tid, SIGINT);

		case SIGKILL:	//End entire process
						kill(thread_l_head->tid, SIGKILL);

		default:	return EINVAL;	//sig is not a valid signal number
	}
	return errno;
}

