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
typedef struct thread_lock_t {
	int mut_lock;
}thread_lock_t;

/*mutex attributes structure*/
typedef struct thread_attr_lock {
}thread_attr_lock;


/*
	Thread library interface for user
*/
int thread_create(thread_t *t, const thread_attr_t * attr, void * (*start_function)(void *), void *arg); // provide option to use a desired mapping.

int thread_join(thread_t thread, void **retval);

void thread_exit(void *retval);

thread_t thread_self(void);

int thread_lock_init(thread_lock_t *mutex, const thread_attr_lock *mutexattr);

int test(thread_lock_t *mutex);

int thread_lock(thread_lock_t *mutex);

int thread_unlock(thread_lock_t *mutex);

int thread_kill(thread_t threads, int sig);


