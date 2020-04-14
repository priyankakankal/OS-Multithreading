# OS-Multithreading
Implementing multi-threading library

This include two libraries using:
1. One-one model
2. Many-one model

Data Structures:
---------------
Double ended Queue for the imeplementation of 
FIFO scheduling of threads.

1.One-One Model:
---------------
Following eight functions are implemented:
1. thread_create() : Clone is used.
2. thread_join()
3. thread_exit()
4. thread_kill() : All signals are handled as in pthread_kill.
5. thread_lock_init() : For initialization of lock.
6. thread_lock()
7. thread_unlock()
8. thread_self(): Will return the thread-id of calling thread.

The prototype of this functions is included in thread.h file.

To Run One-one model:
---------------------------
1. Go to "one-one" folder.
2. Create test.c file and write your code in it.
3. Include "../thread.h" in your test.c file.
4. Run command "make".
5. Run "./test".

All test-files are included in test-files/one-one/ along with the screenshots of the output.
Output of one-one/test.c is shown in screenshot test-files/one-one/test.png.


2.Many-One Model:
---------------
Following eight functions are implemented:
1. thread_create()
2. thread_join()
3. thread_exit()
4. thread_kill() : Signals SIGSTOP, SIGCONT, SIGTERM, SIGHUP, SIGINT and SIGKILL are handled.
5. thread_lock_init() : For initialization of lock.
6. thread_lock()
7. thread_unlock()
8. thread_self(): Will return the thread-id of calling thread.

The prototype of this functions is included in thread.h file.

To Run Many-one model:
---------------------------
1. Go to "many-one" folder.
2. Create test.c file and write your code in it.
3. Include "../thread.h" in your test.c file.
4. Run command "make".
5. Run "./test".

All test-files are included in test-files/many-one/ along with the screenshots of the output.
Output of many-one/test.c is shown in screenshot test-files/many-one/test.png.