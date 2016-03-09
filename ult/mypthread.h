#ifndef H_MYPTHREAD
#define H_MYPTHREAD

#ifndef _XOPEN_SOURCE		// address compilation issues on Mac OS
#define _XOPEN_SOURCE 600
#endif

#define MEM 64000

#include <stdlib.h>
#include <ucontext.h>
#include <stdio.h>
#include <errno.h>

static int tid = 0;			// incremented for each new thread

// tracks active thread. 
// currActive = -1 means ONLY main is touched. 
static int currActive = -1;		


typedef enum {
	untouched,	// thread has not been created
	ready,		// thread has been created
	running,	// thread is active (only one thread is active at a time)
	blocked,	// thread is blocked and waiting . . .
	zombie,		// thread has finished executing 
} state;

// Types
typedef struct {
	// Define any fields you might need inside here.
	int pid;	// unique thread id
} mypthread_t;

typedef struct {
	// Define any fields you might need inside here.
} mypthread_attr_t;

typedef struct {
	int pid;				// unique thread id 
	state status;			// current status of thread
	ucontext_t mycontext;	// context associated with thread

	// so that if a thread is joinable, its calling thread is unblocked
	int wait_pid;			// waiting (if any) thread : -1 OR pid
	void * retval;			// pointer to return value


} mypthread_info_t;

// Functions
int mypthread_create(mypthread_t *thread, const mypthread_attr_t *attr,
			void *(*start_routine) (void *), void *arg);

void mypthread_exit(void *retval);

int mypthread_yield(void);

int mypthread_join(mypthread_t thread, void **retval);


/* Don't touch anything after this line.
 *
 * This is included just to make the mtsort.c program compatible
 * with both your ULT implementation as well as the system pthreads
 * implementation. The key idea is that mutexes are essentially
 * useless in a cooperative implementation, but are necessary in
 * a preemptive implementation.
 */

typedef int mypthread_mutex_t;
typedef int mypthread_mutexattr_t;

static inline int mypthread_mutex_init(mypthread_mutex_t *mutex,
			const mypthread_mutexattr_t *attr) { return 0; }

static inline int mypthread_mutex_destroy(mypthread_mutex_t *mutex) { return 0; }

static inline int mypthread_mutex_lock(mypthread_mutex_t *mutex) { return 0; }

static inline int mypthread_mutex_trylock(mypthread_mutex_t *mutex) { return 0; }

static inline int mypthread_mutex_unlock(mypthread_mutex_t *mutex) { return 0; }

#endif
