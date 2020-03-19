#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include "thread.h"

int thread_create(thread_t *t, const thread_attr_t * attr, void * (*start_function)(void *), void *arg) {
	//one-one and many-one model
	return 0;
}

int thread_join(thread_t thread, void **retval) {
}


/*for testing only*/
int main() {
	return 0;
}
