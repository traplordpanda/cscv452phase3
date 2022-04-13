/* ------------------------------------------------------------------------
   Kyle AuBuchon & Juan Gonzalez
   phase3
   Applied Technology
   College of Applied Science and Technology
   The University of Arizona
   CSCV 452
   ------------------------------------------------------------------------ */\
#include <stdlib.h>
#include <usyscall.h>
#include <string.h>
#include <libuser.h>

#include <usloss.h>
#include <phase1.h>
#include <phase2.h>
#include <phase3.h>
#include <sems.h>

int start2(char *); 
int  spawn_real(char *name, int (*func)(char *), char *arg,
                int stack_size, int priority);
int  wait_real(int *status);

/* Phase 3 -- User Function Prototypes */
extern int Spawn(char *name, int (*func)(char *), char *arg,
                 int stack_size, int priority, int *pid);

extern int Wait(int *pid, int *status);
extern void Terminate(int status);
extern void GetTimeofDay(int *tod);
extern void CPUTime(int *cpu);
extern void getPID(int *pid);
extern int SemCreate(int value, int *semaphore);
extern int SemP(int semaphore);
extern int Semv(int semaphore);
extern int SemFree(int semaphore);

extern int spawn_real(char *name, int (*func)(char *), char *arg,
                      int stack_size, int priority);

extern int wait_real(int *status);
extern void terminate_real(int exit_code);
extern int semcreate_real(int init_value);
extern int semp_real(int semaphore);
extern int semv_real(int semaphore);
extern int semfree_real(int semaphore);
extern int gettimeofday_real(int *time);
extern int cputime_real(int *time);
extern int getPID_real(int *pid);
extern int start3(char *arg);

void check_kernel_mode(char * proc);

/* Global Data Structs */

ProcessTable procTable[MAXPROC]; 

/* Global Data Structs */
int start2(char *arg)
{
    int		pid;
    int		status;
    /*
     * Check kernel mode here.
     */
    check_kernel_mode("start2\n");

    /*
     * Data structure initialization as needed...
     * Init proc table as EMPTY 
     */
    for (int i = 0; i < MAXPROC; i++){
		procTable[i].status = EMPTY;
	}

    /* Init Sys Call Handler*/
    for (int i = 0; i < MAXSYSCALLS; i++)
    {
        sys_vec[i] = NULL;
    }
    
    /*
     * Create first user-level process and wait for it to finish.
     * These are lower-case because they are not system calls;
     * system calls cannot be invoked from kernel mode.
     * Assumes kernel-mode versions of the system calls
     * with lower-case names.  I.e., Spawn is the user-mode function
     * called by the test cases; spawn is the kernel-mode function that
     * is called by the syscall_handler; spawn_real is the function that
     * contains the implementation and is called by spawn.
     *
     * Spawn() is in libuser.c.  It invokes usyscall()
     * The system call handler calls a function named spawn() -- note lower
     * case -- that extracts the arguments from the sysargs pointer, and
     * checks them for possible errors.  This function then calls spawn_real().
     *
     * Here, we only call spawn_real(), since we are already in kernel mode.
     *
     * spawn_real() will create the process by using a call to fork1 to
     * create a process executing the code in spawn_launch().  spawn_real()
     * and spawn_launch() then coordinate the completion of the phase 3
     * process table entries needed for the new process.  spawn_real() will
     * return to the original caller of Spawn, while spawn_launch() will
     * begin executing the function passed to Spawn. spawn_launch() will
     * need to switch to user-mode before allowing user code to execute.
     * spawn_real() will return to spawn(), which will put the return
     * values back into the sysargs pointer, switch to user-mode, and 
     * return to the user code that called Spawn.
     */
    pid = spawn_real("start3", start3, NULL, 4*USLOSS_MIN_STACK, 3);
    pid = wait_real(&status);

} /* start2 */

void check_kernel_mode(char * proc) {
    if ((PSR_CURRENT_MODE & psr_get()) == 0) {
        console("check_kernel_mode(): called while in user mode by process %s. Halting...\n", proc);
        halt(1);
    }
}
/* --------------------------------------------------------------------------------
   Name - Spawn
   Purpose - Create user level process
   Parameters - arg1 - address of function to spawn
                arg2 - parameter passed to spawned functions
                arg3 - stack size in bytes
                arg4 - priority
                arg5 - characters string containing process's name
   Returns -    arg1 - PID of newly created process; -1 if process could not be completed
                arg4 - -1 if illegal values, 0 otherwise
   Side Effects -  if system is in usermode, print appropriate error and halt.
   -------------------------------------------------------------------------------- */
static void spawn(sysargs *args_ptr){
    /*
    int (*func)(char *);
    char *arg;
    int stack_size;
    int pid;

    if (is_zapped() ){//should terminate proc}

    func = args_ptr -> arg1;
    arg = args_ptr -> arg2;
    stack_size = (int) args_ptr -> arg3;
    //more code to extract system call arguments as well as exception handling
    if ((long) args->number != SYS_SPAWN){
		args->arg4 = (void *) -1;
		return;
	}

	if ((long) args->arg3 < USLOSS_MIN_STACK){
		args->arg4 = (void *) -1;
		return;
	}
	if ((long) args->arg4 > MINPRIORITY || (long) args->arg4 < MAXPRIORITY){
		args->arg4 = (void *) -1;
		return;
	}

	pid = spawn_real((char *) args->arg5, args->arg1, args->arg2,
			         (long) args->arg3,(long) args->arg4);
    //call another function to modularize the code better
    kid_pid = spawn_real(name, func, arg, stack_size, priority);
    args_ptr
    */
}