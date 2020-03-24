#include<malloc.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define FALSE 0
#define TRUE 1

/* Possible states of thread */
#define RUNNING 0
#define READY 	1 /* Ready to be scheduled */
#define BLOCKED 2 /* Waiting on Join */
#define DEFUNCT 3 /* Dead */

typedef pid_t thread_t;

typedef struct thread_attr_t {
	unsigned long stack_size;     /* Stack size to be used by this thread. Default is SIGSTKSZ */
} thread_attr_t;


/*Thread Control Block structure */
typedef struct thread_struct {
	thread_t tid; 			/* The thread-id of the thread */
	int state; 			/* the state in which the corresponding thread will be. */
	void * (*start_func) (void *); 	/* The func pointer to the thread function to be executed. */
	void *arg; 			/* The arguments to be passed to the thread function. */
	void *returnValue; 			/* The return value that thread returns. */
	struct thread_struct *blockedForJoin; 	/* Thread blocking on this thread */
	struct thread_struct *prev, *next; 
} thread_struct;


/*
	Thread library interface for user
*/
int thread_create(thread_t *t, const thread_attr_t * attr, void * (*start_function)(void *), void *arg); // provide option to use a desired mapping.

int thread_join(thread_t thread, void **retval);

void thread_exit(void *retval);
/*

thread_lock(); // a spinlock
thread_unlock();  // spin-unlock
thread_kill();

*/


// Global pointer to the head node in the queue of Thread Structure
thread_struct *thread_l_head;

/*
	Internal thread function
*/
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

