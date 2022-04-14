/* ------------------------------------------------------------------------
   Kyle AuBuchon & Juan Gonzalez
   phase3
   Applied Technology
   College of Applied Science and Technology
   The University of Arizona
   CSCV 452
   ------------------------------------------------------------------------ */

#ifndef PHASE_3_SEMS_H_
#define PHASE_3_SEMS_H_

// define priorities
#define MINPRIORITY 5
#define MAXPRIORITY 1

// define start process pids
#define START2_PID 3
#define START3_PID 4

// define proc table status
#define EMPTY 0
#define ACTIVE 1
#define WAIT_BLOCK 2

#endif /* PHASE_3_SEMS_H_ */

typedef struct ProcessTable * ProcessTablePtr;
typedef struct semaphore semaphore;
typedef struct procQueue procQueue;

/* 
 * phase 3 proc table struct
 */
typedef struct ProcessTable {
    ProcessTablePtr childProcessPtr;
    ProcessTablePtr nextSiblingPtr;
    ProcessTablePtr parentPtr;
    ProcessTablePtr nextSemaphoreBlocked;
    char name[MAXNAME];
    char startArg[MAXARG];
    short pid;
    int priority;
    int( * func)(char * );
    unsigned int stackSize;
    int status;
    int mboxID;
}
ProcessTable;

/*needed for Semaphore implementation*/
struct procQueue {
    ProcessTablePtr head;
    ProcessTablePtr tail;
    int size;
    int type;
};
/* 
 * Semaphore struct
 */

struct semaphore {
    int id;
    int value;
    int start;
    procQueue blocked;
    int priv_mboxID;
    int mutex_mboxID;
};