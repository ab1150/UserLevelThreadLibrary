#include "mypthread.h"


mypthread_info_t table[512];
typedef void (*func)(void);  // Declare typedef

int mypthread_create(mypthread_t *thread, const mypthread_attr_t *attr,
			void *(*start_routine) (void *), void *arg)
{
	// error checking
	if(thread == NULL || start_routine == NULL){
		printf("Error: Null pointer to thread or start routine\n");
		return 1;
	}

	// only execute upon the first call of mypthread_create()
	static char first = 0;
	if(first == 0){
		// initialize all mypthread_t statuses to untouched
		int i;
		for(i = 0 ; i < 511 ; i++){
			table[i].status = untouched;
		}
		// initialize the main thread (tid = 0 at this point)
		table[tid].pid = tid;
		table[tid].status = running;
		getcontext(&(table[tid].mycontext));
		table[tid].wait_pid = -1;

		// initialize currently active thread to main
		currActive = tid;
		
		// increment
		tid++;
		first++;
	}

	// create the entry of the new thread
	thread->pid = tid;
	table[tid].pid = tid;
	table[tid].status = ready;
	getcontext(&(table[tid].mycontext));
	table[tid].wait_pid = -1;

	// set up ucontext_t

	table[tid].mycontext.uc_link = 0;		

	// set up stack

	table[tid].mycontext.uc_stack.ss_sp = malloc(MEM);
	table[tid].mycontext.uc_stack.ss_size = MEM;
	table[tid].mycontext.uc_stack.ss_flags = 0;


	// makecontext

	if(arg == NULL){
		makecontext( &(table[tid].mycontext) , (func) start_routine, 0);
	}else{
		makecontext( &(table[tid].mycontext) , (func) start_routine, 1, arg);
	}

	// increment
	tid++;

	return 0;

}

// differs from exit in that currently active thread can be
// scheduled again
int mypthread_yield(void){

	// return to main
	if(currActive < 0){
		return 0;
	}

	// find the next thread to run
	int next;
	if(currActive < 512 - 1){
		next = currActive + 1;
	}else{
		next = 0;
	}

	// it will always find a thread that is ready (may be itself)
	while(table[next].status != ready){
	
		if(next == 512 - 1){
			next = 0;
		}else{
			next++;
		}
	}

	// case i: next thread is itself 
	// Note: only main can be the only thread running.
	// Once main exits, the entire process finishes.
	if(next == currActive){
		return 0;
	}

	// save the previous active thread

	int prev = currActive;
	currActive = next;

	// make changes to the statuses

	// if currActive is blocked, it will remain blocked
	// if currActive is running, it will be ready 

	if(table[prev].status = running){
		table[prev].status = ready;
	}

	table[currActive].status = running;

	swapcontext(&(table[prev].mycontext),&(table[currActive].mycontext));

	return 0;
}

// note: retval must be dynamically allocated. or else the pointer will be lost.
void mypthread_exit(void *retval){
	// exit out of active thread

	// exit out of main
	if(currActive < 0){
		exit(0);
	}


	if( table[currActive].status != running ){
		printf("hi\n");
		printf("Thread in question is %d status is %d\n",currActive,table[currActive].status );
		printf("BIG ERROR malfunction terrible idea\n");

		return;
	}

	// storing pointer in the table
	table[currActive].retval = retval;

	int wait = table[currActive].wait_pid;
	// unblocking the waiting thread
	if( wait >= 0){
		table[wait].status = ready;
	}

	// find the next thread to run
	int next;
	if(currActive < 512 - 1){
		next = currActive + 1;
	}else{
		next = 0;
	}


	/* schedule next thread here
	 *	case i) no thread is found (that is not itself) , so exit the process.
	 *			
	 *	case ii) thread is found, so swap contexts
	 */
	while(table[next].status != ready){
		
		if(next == 512 - 1){
			next = 0;
		}else{
			next++;
		}

		// case i) . terminate current thread and entire process
		if(next == currActive){
			table[currActive].status = zombie;
			//currActive = -1;
			exit(0);
		}
	}

	// case ii)

	int prev = currActive;

	table[currActive].status = zombie;
	currActive = next;

	table[next].status  = running; 

	/* Note: The previously active thread will
	 * NEVER be scheduled again because its status 
	 * has been altered to zombie.
	 * (only 'ready' threads are scheduled)
	 */
	swapcontext( &(table[prev].mycontext) ,&(table[currActive].mycontext));


	return;

}

int mypthread_join(mypthread_t thread, void **retval){

	// if(thread == NULL){
	// 	printf("Error: Must specify a joinable thread\n");
	// 	return -1;
	// }

	// check existence
	if(thread.pid < 0){
		printf("Error: Thread does not exist.\n");
		return -1;
	}

	// check if thread is joing on itself
	if(thread.pid == currActive){
		printf("Error: Thread cannot join on itself\n");
		return -1;
	}

	// if thread has not terminated already
	if(table[thread.pid].status != zombie){

		// update that the joinable thread has a waiting thread
		table[thread.pid].wait_pid = currActive;
		// block the current thread
		table[currActive].status = blocked;

		mypthread_yield();
	}

	// set the double pointer retval
	if(retval != NULL){
		*retval = (table[thread.pid].retval);
	}

	return 0;
}

