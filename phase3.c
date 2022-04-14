/* ------------------------------------------------------------------------
   Kyle AuBuchon & Juan Gonzalez
   phase3
   Applied Technology
   College of Applied Science and Technology
   The University of Arizona
   CSCV 452
   ------------------------------------------------------------------------ */
#include <stdlib.h>

#include <usyscall.h>

#include <string.h>

#include <libuser.h>

#include <usloss.h>

#include <phase1.h>

#include <phase2.h>

#include <phase3.h>

#include <sems.h>

/* Phase 3 -- Function Prototypes */

int start2(char * );
extern int start3(char * arg);
static void spawn(sysargs * args);
int spawn_real(char * name, int( * func)(char * ), char * arg, int stack_size, int priority);
int spawn_launch(char * arg);

int wait_real(int * status);
void wait1(sysargs * args);

void terminate(sysargs * args);
void nullsys3(sysargs * args);

void addChildToList(ProcessTablePtr child);
void removeChildFromList(ProcessTablePtr process);

void getPID(sysargs * args);
void getTimeOfDay(sysargs * args);
void cpuTime(sysargs * args);

void set_user_mode();
void check_kernel_mode(char * proc);

/* Global Data */
ProcessTable procTable[MAXPROC];
/* Global Data */

/* 
            ----------- start2 ---------------- 
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
int start2(char * arg) {
    int pid;
    int status;
    /*
     * Check kernel mode here.
     */
    check_kernel_mode("start2\n");

    /*
     * Data structure initialization as needed...
     * Init proc table as EMPTY 
     */
    for (int i = 0; i < MAXPROC; i++) {
        procTable[i].status = EMPTY;
    }

    /* Init Sys Call Handler*/
    for (int i = 0; i < MAXSYSCALLS; i++) {
        sys_vec[i] = NULL;
    }

    /* Init Sys call vector */

    sys_vec[SYS_SPAWN] = spawn;
    sys_vec[SYS_WAIT] = wait1;
    sys_vec[SYS_TERMINATE] = terminate;
    sys_vec[SYS_GETPID] = getPID;
    sys_vec[SYS_GETTIMEOFDAY] = getTimeOfDay;
    sys_vec[SYS_CPUTIME] = cpuTime;

    pid = spawn_real("start3", start3, NULL, 4 * USLOSS_MIN_STACK, 3);
    //check pid value
    if (pid < 0) {
        quit(pid);
    }

    pid = wait_real( & status);

    //check pid value
    if (pid < 0) {
        quit(pid);
    }
    quit(0);
    return 0;

} /* start2 */

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
static void spawn(sysargs * args) {
    long kid_pid;

    //check for invalid args
    if ((long) args -> number != SYS_SPAWN) {
        args -> arg4 = (void * ) - 1;
        return;
    }

    if ((long) args -> arg3 < USLOSS_MIN_STACK) {
        args -> arg4 = (void * ) - 1;
        return;
    }
    if ((long) args -> arg4 > MINPRIORITY || (long) args -> arg4 < MAXPRIORITY) {
        args -> arg4 = (void * ) - 1;
        return;
    }

    kid_pid = spawn_real((char * ) args -> arg5, args -> arg1, args -> arg2,
        (long) args -> arg3, (long) args -> arg4);

    args -> arg1 = (void * ) kid_pid; // id of the newly created process, -1 if failed
    args -> arg4 = (void * ) 0;
    set_user_mode();
} /* spawn */
/* --------------------------------------------------------------------------------
   Name - spawn_real
   fork1 to create a process that runs a start function
   process runs at user mode - Maintain the parent-child relationship at phase 3 process table
   -------------------------------------------------------------------------------- */
int spawn_real(char * name, int( * func)(char * ), char * arg, int stackSize, int priority) {
    int kidpid;
    /*synchonize using a mailbox*/
    int mboxID;

    kidpid = fork1(name, spawn_launch, arg, stackSize, priority);

    /*value check pid*/
    if (kidpid < 0) {
        return kidpid;
    }

    /*Check parent process priority
     *if higher init proctable
     *if not higher
     */
    if (procTable[kidpid % MAXPROC].status == EMPTY) {
        mboxID = MboxCreate(0, 0);
        procTable[kidpid % MAXPROC].mboxID = mboxID;
        procTable[kidpid % MAXPROC].status = ACTIVE;
    } else {
        mboxID = procTable[kidpid % MAXPROC].mboxID;
    }

    /*init child in proctable*/

    strcpy(procTable[kidpid % MAXPROC].name, name);
    procTable[kidpid % MAXPROC].pid = kidpid;
    procTable[kidpid % MAXPROC].priority = priority;
    procTable[kidpid % MAXPROC].func = func;
    procTable[kidpid % MAXPROC].childProcessPtr = NULL;
    procTable[kidpid % MAXPROC].nextSiblingPtr = NULL;
    //--BUGFIX-- procTable[kidpid % MAXPROC].nextSemaphoreBlocked = NULL; // --BUGFIX--
    procTable[kidpid % MAXPROC].stackSize = stackSize;

    if (arg == NULL) {
        procTable[kidpid % MAXPROC].startArg[0] = 0;
    } else {
        strcpy(procTable[kidpid % MAXPROC].startArg, arg);
    }

    /* block start2 init in proc table - not clean */
    if (getpid() != START2_PID) {
        procTable[kidpid % MAXPROC].parentPtr = & procTable[getpid() % MAXPROC];
        addChildToList( & procTable[kidpid % MAXPROC]);
    }

    MboxCondSend(mboxID, NULL, 0);

    return kidpid;
} /* spawn_real */
/* --------------------------------------------------------------------------------
   Name - spawn_launch
   used by fork to exec user level proc
   -------------------------------------------------------------------------------- */

int spawn_launch(char * arg) {
    int my_pid = getpid();
    int result; // 
    int mboxID;

    /*sanity check for priorities*/
    if (procTable[my_pid % MAXPROC].status == EMPTY) {
        procTable[my_pid % MAXPROC].status = ACTIVE;
        mboxID = MboxCreate(0, 0);
        procTable[my_pid % MAXPROC].mboxID = mboxID;
        MboxReceive(mboxID, NULL, 0);
    }

    /*make sure process is not zapped*/
    if (!is_zapped()) {
        set_user_mode();
        result = procTable[my_pid % MAXPROC].func(procTable[my_pid % MAXPROC].startArg);
        Terminate(result); // BUGFIX
    } else {
        Terminate(0);
    }
    set_user_mode();

    /*call function*/
    result = procTable[my_pid % MAXPROC].func(procTable[my_pid % MAXPROC].startArg);

    Terminate(result);
    return 0;
} /* spawn_launch */

/* --------------------------------------------------------------------------------
   Name - wait1
   Wait for a child process to terminate. If the process is zapped while it is waiting, the process
   should be terminated using the equivalent of the Terminate syscall.
   arg1: process id of the terminating child.
   arg2: the termination code of the child.
   arg4: -1 if the process has no children, 0 otherwise.
   -------------------------------------------------------------------------------- */
void wait1(sysargs * args) {
    int status;
    long deadChildPID;

    if ((long) args -> number != SYS_WAIT) {
        args -> arg2 = (void * ) - 1;
        return;
    }

    /*grab termination code*/
    deadChildPID = wait_real( & status);

    procTable[getpid() % MAXPROC].status = ACTIVE;

    /*check if proc had children*/
    if (deadChildPID == -2) {
        args -> arg1 = (void * ) 0;
        args -> arg2 = (void * ) - 2;
        args -> arg4 = (void * ) - 1;
    } else {
        args -> arg1 = (void * ) deadChildPID;
        args -> arg2 = ((void * )(long) status);
        args -> arg4 = (void * ) 0;
    }

    set_user_mode();
} /* wait */

/* --------------------------------------------------------------------------------
   Name - wait real
   used to set proc table and call join()
   returns PID of dead child
   -------------------------------------------------------------------------------- */
int wait_real(int * status) {
    procTable[getpid() % MAXPROC].status = WAIT_BLOCK;
    return join(status);
} /* wait_real */

/* --------------------------------------------------------------------------------
   Name - termiante
   Terminates the invoking process and all of its children and synchronizes with its parent’s Wait
   system call.
   -------------------------------------------------------------------------------- */
void terminate(sysargs * args) {
    /*create proc table*/
    ProcessTablePtr parent = & procTable[getpid() % MAXPROC];

    /*zap each child proc*/
    if (parent -> childProcessPtr != NULL) {
        while (parent -> childProcessPtr != NULL) {
            zap(parent -> childProcessPtr -> pid);
        }
    }
    if (parent -> pid != START3_PID && parent -> parentPtr != NULL) {
        removeChildFromList( & procTable[getpid() % MAXPROC]);
    }

    /*user-level process terminates*/
    parent -> status = EMPTY;
    quit((int)(long) args -> arg1);
}

/* Check USLOSS mode - if not kernel mode call halt*/
void check_kernel_mode(char * proc) {
    if ((PSR_CURRENT_MODE & psr_get()) == 0) {
        console("check_kernel_mode(): called while in user mode by process %s. Halting...\n", proc);
        halt(1);
    }
}

void nullsys3(sysargs * args) {
    console("nullsys(): Invalid syscall %d. Terminating...\n", args -> number);
    Terminate(1);
}

/* return getpid() value*/
void getPID(sysargs * args) {
    args -> arg1 = ((void * )(long) getpid());
    set_user_mode();
}

void getTimeOfDay(sysargs * args) {
    args -> arg1 = ((void * )(long) sys_clock());
    set_user_mode();
}

void cpuTime(sysargs * args) {
    args -> arg1 = ((void * )(long) readtime());
    set_user_mode();
}

void set_user_mode() {
    psr_set(psr_get() & 14);
}

/* add child to parent proc table */
void addChildToList(ProcessTablePtr proc) {
    ProcessTablePtr parent = & procTable[getpid() % MAXPROC];

    if (parent -> childProcessPtr == NULL) {
        parent -> childProcessPtr = proc;
    } else {

        ProcessTablePtr temp = parent -> childProcessPtr;
        while (temp -> nextSiblingPtr != NULL) {
            temp = temp -> nextSiblingPtr;
        }
        temp -> nextSiblingPtr = proc;
    }

}

/* remove child from parent proc table */
void removeChildFromList(ProcessTablePtr proc) {
    ProcessTablePtr temp = proc;

    if (proc == proc -> parentPtr -> childProcessPtr) {
        proc -> parentPtr -> childProcessPtr = proc -> nextSiblingPtr;
    } else {
        temp = proc -> parentPtr -> childProcessPtr;
        while (temp -> nextSiblingPtr != proc) {
            temp = temp -> nextSiblingPtr;
        }
        temp -> nextSiblingPtr = temp -> nextSiblingPtr -> nextSiblingPtr;
    }
}