#include<malloc.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <errno.h>

#define FALSE 0
#define TRUE 1

/* Possible states of thread */
#define RUNNING 0
#define READY 	1 /* Ready to be scheduled */
#define BLOCKED 2 /* Waiting on Join */
#define DEAD 3 /* Dead */

typedef pid_t thread_t;

typedef struct thread_attr_t {
	unsigned long stack_size;     /* Stack size to be used by this thread. Default is SIGSTKSZ */
} thread_attr_t;


/*spinlock mutex structure*/
typedef struct thread_mutex_t {
	int mut_lock;
}thread_mutex_t;

/*mutex attributes structure*/
typedef struct thread_mutexattr_t {
}thread_mutexattr_t;


/*
	Thread library interface for user
*/
int thread_create(thread_t *t, const thread_attr_t * attr, void * (*start_function)(void *), void *arg); // provide option to use a desired mapping.

int thread_join(thread_t thread, void **retval);

void thread_exit(void *retval);

thread_t thread_self(void);
/*

thread_lock(); // a spinlock
thread_unlock();  // spin-unlock
*/

int thread_mutex_init(thread_mutex_t *mutex, const thread_mutexattr_t *mutexattr);

int test(thread_mutex_t *mutex);

int thread_mutex_lock(thread_mutex_t *mutex);

int thread_mutex_unlock(thread_mutex_t *mutex);

int thread_kill(thread_t threads, int sig);



