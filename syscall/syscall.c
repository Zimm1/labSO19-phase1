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

//SYS2
HIDDEN int createProcess(state_t* statep, int priority, pcb_t* cpid){
    cpid = allocPcb();

    if(cpid == NULL){
        return FALSE;
    }

    cpid->tutor = FALSE;
    cpid->cpu_time = 0;
    cpid->original_priority = cpid->priority = priority;
    copyState(statep, &cpid->p_s);

    insertChild(currentProcess, cpid);
    insertProcQ(&readyQueue, cpid);

    return TRUE;
}

/**
  * @brief (SYS3) Terminates a process and all of its children.
  * @param pcb : Process to terminate.
  * @return void.
 */
HIDDEN int terminateProcess(pcb_t* pcb) {
    if(pcb == NULL || pcb == 0){
        pcb = currentProcess;
    } else if (!isParent(pcb, currentProcess)){
        return -1;
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

    return 0;
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

//SYS6
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

//SYS8
void setTutor(){
    if(currentProcess != NULL){
        currentProcess->tutor = TRUE;
    }
}

//SYS10
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
            case GETPID:
                getPid((pcb_t*) a1, (pcb_t*) a2);

            default:
                PANIC();
                break;
        }

        schedule();
    } else {
        PANIC();
    }
}