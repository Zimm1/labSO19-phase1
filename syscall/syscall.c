#include <umps/libumps.h>

#include "syscall.h"
#include "scheduler/scheduler.h"
#include "tests/p2/main.h"
#include "pcb/pcb.h"
#include "asl/asl.h"
#include "utils/types_rikaya.h"
#include "utils/const_rikaya.h"
#include "utils/utils.h"

state_t* sysbp_old = (state_t*) SYSBK_OLDAREA;

/**
  * @brief (SYS2) Creates a new process.
  * @param cpid : pointer to the process created, if created without error.
  * @return Returns -1 if it's impossible to create the new process.
 */
HIDDEN void createProcess(state_t* statep, int priority, pcb_t* cpid){
    cpid = allocPcb();

    if(cpid == NULL){
        currentProcess->p_s.reg_a1 = -1;
    }

    cpid->tutor = FALSE;
    cpid->cpu_time = 0;
    cpid->original_priority = cpid->priority = priority;
    copyState(statep, &cpid->p_s);

    insertChild(currentProcess, cpid);
    insertProcQ(&readyQueue, cpid);

    currentProcess->p_s.reg_a1 = 0;
}

/**
  * @brief (SYS3) Terminates a process and adds his children to the TUTOR.
  * @param pcb : process to terminate, if the value of pcb is NULL terminates the current process.
  * @return Returns -1 if the process to terminate don't come from currentProcess.
 */
HIDDEN void terminateProcess(pcb_t* pcb) {
    if(pcb == NULL || pcb == 0){
        pcb = currentProcess;
        currentProcess = NULL;
    } else if (!isParent(pcb, currentProcess)){
        currentProcess->p_s.reg_a1 = -1;
        return;
    }
    
    if(pcb != NULL){
        pcb_t* tutor = getTUTOR(pcb->p_parent);
        while(!emptyChild(pcb)){
            pcb_t* child = removeChild(pcb);
            insertChild(tutor, child);
        }

        outProcQ(&readyQueue, pcb);
        freePcb(pcb);
    }

    currentProcess->p_s.reg_a1 = 0;
}

// SYS4
void verhogen(int *sem) {
    (*sem)++;
    pcb_t* first = removeBlocked(sem);

    if (first != NULL) {
        first->p_semkey = NULL;
        insertProcQ(&readyQueue, first);
    }
}

// SYS5
void passeren(int *sem) {
    (*sem)--;

    if ((*sem)<0) {
        currentProcess->p_semkey = sem;
        currentProcess->cpu_time += getTODLO() - process_TOD;

        insertBlocked(sem, currentProcess);
        currentProcess = NULL;
    }
}

/**
  * @brief (SYS6) Suspends the process until the next tick of the system clock (100 ms).
  * @return void.
 */
void waitClock(){
    passeren(&semPseudoClock);
}


// SYS7
void doIO(unsigned int command, int* reg) {
    *(reg + 3) = command;

    // TODO CHANGE
    int line = 8;
    int dev = 0;

    memaddr *semaphoreDevice = getSemDev(line, dev);
    memaddr *statusReg = getKernelStatusDev(line, dev);

    if((*statusReg) != 0){
        currentProcess->p_s.reg_a1 = (*statusReg);
        (*statusReg) = 0;
    } else {
        passeren((int *) semaphoreDevice);
    }
}

/**
  * @brief (SYS8) Marks the calling process as TUTOR.
  * @return void.
 */
void setTutor(){
    if(currentProcess != NULL){
        currentProcess->tutor = TRUE;
    }
}

/**
  * @brief (SYS9) Sets the handler for a exception of a certain type.
  * The possible values of type are:
  * 0: SYS/Bp exception
  * 1: TLB exception
  * 2: PgmTrap exception
  * @return 0 if the SYSCALL ends without error.
 */
void specPassup(int type, state_t* oldArea, state_t* newArea){
    if(currentProcess->exceptionVector[type*2] != NULL){
        terminateProcess(currentProcess);
        currentProcess->p_s.reg_a1 = -1;
    } else {
        currentProcess->exceptionVector[type*2] = oldArea;
        currentProcess->exceptionVector[type*2+1] = newArea;
        currentProcess->p_s.reg_a1 = 0;
    }
}

/**
  * @brief (SYS10) Sets the pointer of the currentProcess to pid (if pid != NULL) and
  *     the pointer of the parent process to ppid (if ppid != NULL).
  * @return void.
 */
void getPid(pcb_t* pid, pcb_t* ppid){
    if(pid != NULL){
        pid = currentProcess;
    }
    if(ppid != NULL){
        ppid = pid->p_parent;
    }
}

/**
  * @brief System calls and breakpoints handler, checks the cause and calls the right sub-handler
  * @return void.
 */
void sysBpHandler() {
    copyState(sysbp_old, &currentProcess->p_s);
    currentProcess->p_s.pc_epc += WORD_SIZE;

    unsigned int cause = CAUSE_EXCCODE_GET(sysbp_old->cause);
    unsigned int a0 = sysbp_old->reg_a0;
    unsigned int a1 = sysbp_old->reg_a1;
    unsigned int a2 = sysbp_old->reg_a2;
    unsigned int a3 = sysbp_old->reg_a3;

    if (cause == EXC_SYS) {
        switch(a0) {
            case GETCPUTIME:
                break;

            case CREATEPROCESS:
                createProcess((state_t *) a1, (int) a2, (pcb_t *) a3);
                break;

            case TERMINATEPROCESS:
                terminateProcess((pcb_t*) a1);
                break;

            case VERHOGEN:
                verhogen((int *) a1);
                break;

            case PASSEREN:
                passeren((int *) a1);
                break;

            case WAITCLOCK:
                waitClock();
                break;

            case WAITIO:
                doIO(a1, (int *) a2);
                break;

            case SPECPASSUP:
                specPassup((int) a1 ,(state_t*) a2, (state_t*) a3);
                break;
                
            case GETPID:
                getPid((pcb_t*) a1, (pcb_t*) a2);
                break;
                
            default:
                PANIC();
                break;
        }

        schedule();
    } else {
        PANIC();
    }
}