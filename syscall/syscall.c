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

HIDDEN void getCpuTime(int* user, int* kernel, int* wallclock) {
    if (user != NULL) {
        *user = currentProcess->time_user;
    }

    if (kernel != NULL) {
        *kernel = currentProcess->time_kernel;
    }

    if (wallclock != NULL) {
        *wallclock = GET_TODLOW - currentProcess->time_start;
    }
}

/**
  * @brief (SYS2) Creates a new process.
  * @param cpid : pointer to the process created, if created without error.
  * @return Returns -1 if it's impossible to create the new process.
 */
HIDDEN void createProcess(state_t* statep, int priority, unsigned int* cpid){
    pcb_t* pcb = allocPcb();

    if (pcb == NULL) {
        currentProcess->p_s.reg_a1 = -1;
    }

    pcb->tutor = FALSE;
    pcb->time_start = 0;
    pcb->time_user = 0;
    pcb->time_kernel = 0;
    pcb->original_priority = pcb->priority = priority;
    copyState(statep, &pcb->p_s);

    insertChild(currentProcess, pcb);
    lock(&MUTEX_SCHEDULER);
    insertProcQ(&readyQueue, pcb);
    unlock(&MUTEX_SCHEDULER);

    if (cpid != NULL) {
        *cpid = (unsigned int) pcb;
    }

    currentProcess->p_s.reg_a1 = 0;
}

/**
  * @brief (SYS3) Terminates a process and adds his children to the TUTOR.
  * @param pcb : process to terminate, if the value of pcb is NULL terminates the current process.
  * @return Returns -1 if the process to terminate don't come from currentProcess.
 */

pcb_t* pcb3;
pcb_t* parent;

void addSem(pcb_t* pcb) {
    *pcb->p_semkey =  *pcb->p_semkey + 1;
}

HIDDEN void terminateProcess(pcb_t* pcb) {
    if (pcb == NULL || pcb == currentProcess) {
        pcb = currentProcess;
        currentProcess = NULL;
    } else if (!isParent(pcb, currentProcess)) {
        currentProcess->p_s.reg_a1 = -1;
        return;
    }
    
    if (pcb != NULL) {
        pcb3 = pcb;
        parent = pcb->p_parent;
        pcb_t* tutor = getTUTOR(pcb->p_parent);
        pcb_t* child;
        while (child = removeChild(pcb)) {
            insertChild(tutor, child);
        }

        outProcQ(&readyQueue, pcb);
        if (pcb->p_semkey != NULL) {
            addSem(pcb);
        }
        outBlocked(pcb);
        outChild(pcb);

        freePcb(pcb);
    }

    if (currentProcess != NULL) {
        currentProcess->p_s.reg_a1 = 0;
    }
}

HIDDEN void terminateProcessId(int* pid) {
    terminateProcess(pid != NULL ? (pcb_t*) *pid : NULL);
}

unsigned int MUTEX_PV = 1;

// SYS4
void verhogen(int *sem) {
    lock(&MUTEX_PV);

    (*sem)++;
    pcb_t* first = removeBlocked(sem);

    if (first != NULL) {
        first->p_semkey = NULL;
        first->time_kernel += getTODLO() - process_TOD;
        lock(&MUTEX_SCHEDULER);
        insertProcQ(&readyQueue, first);
        unlock(&MUTEX_SCHEDULER);
    }

    unlock(&MUTEX_PV);
}

// SYS5
void passeren(int *sem) {
    lock(&MUTEX_PV);

    (*sem)--;

    if ((*sem)<0) {
        currentProcess->p_semkey = sem;
        currentProcess->time_kernel += getTODLO() - process_TOD;

        insertBlocked(sem, currentProcess);
        currentProcess = NULL;
    }

    unlock(&MUTEX_PV);
}

/**
  * @brief (SYS6) Suspends the process until the next tick of the system clock (100 ms).
  * @return void.
 */
void waitClock(){
    passeren(&semPseudoClock);
}

memaddr *semaphoreDevice;
int line;
int dev;

// SYS7
void doIO(unsigned int command, int* reg) {
    //*(reg) = command;
    ((devreg_t*)reg)->term.transm_command = command;
    
    line = DEV_LINE((int)reg);
    dev = DEV_NUMBER((int)reg);

    semaphoreDevice = getSemDev(line, dev);
    //memaddr *statusReg = getKernelStatusDev(line, dev);

    //if((*statusReg) != 0){
    //    currentProcess->p_s.reg_a1 = (*statusReg);
    //    (*statusReg) = 0;
    //} else {
    passeren((int *) semaphoreDevice);
    //}
}

unsigned int cause;
unsigned int a0;

/**
  * @brief (SYS8) Marks the calling process as TUTOR.
  * @return void.
 */
void setTutor(){
    if (currentProcess != NULL) {
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
    if (currentProcess->exceptionVector[type*2] != NULL) {
        terminateProcess(currentProcess);
        // currentProcess->p_s.reg_a1 = -1;
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
void getPid(unsigned int* pid, unsigned int* ppid){
    if (pid != NULL) {
        *pid = (unsigned int) currentProcess;
    }

    if (ppid != NULL) {
        *ppid = (unsigned int) currentProcess->p_parent;
    }
}

/**
  * @brief System calls and breakpoints handler, checks the cause and calls the right sub-handler
  * @return void.
 */

unsigned int sysStatus;
unsigned int userMode;

void sysBpHandler() {
    if (currentProcess != NULL) {
        copyState(sysbp_old, &currentProcess->p_s);
        currentProcess->p_s.pc_epc += WORD_SIZE;
        currentProcess->time_user += getTODLO() - process_TOD;
    }
    process_TOD = getTODLO();

    sysStatus = currentProcess->p_s.status;
    userMode = isUserMode(currentProcess);

    if (currentProcess->p_s.reg_a0 <= SYSCALL_MAX && isUserMode(currentProcess)) {
		copyState((state_t*)SYSBK_OLDAREA, (state_t *)PGMTRAP_OLDAREA);

		((state_t *)PGMTRAP_OLDAREA)->cause = CAUSE_EXCCODE_SET(
            CAUSE_EXCCODE_GET(((state_t *)PGMTRAP_OLDAREA)->cause),
            EXC_RESERVEDINSTR
        );
	
        trapHandler();
    }

    cause = CAUSE_EXCCODE_GET(sysbp_old->cause);
    a0 = sysbp_old->reg_a0;
    unsigned int a1 = sysbp_old->reg_a1;
    unsigned int a2 = sysbp_old->reg_a2;
    unsigned int a3 = sysbp_old->reg_a3;

    if (cause == EXC_SYS) {
        switch(a0) {
            case GETCPUTIME:
                getCpuTime((int *) a1, (int *) a2, (int *) a3);
                break;

            case CREATEPROCESS:
                createProcess((state_t *) a1, (int) a2, (unsigned int*) a3);
                break;

            case TERMINATEPROCESS:
                terminateProcessId((int*) a1);
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

            case SETTUTOR:
                setTutor();
                break;

            case SPECPASSUP:
                specPassup((int) a1 ,(state_t*) a2, (state_t*) a3);
                break;
                
            case GETPID:
                getPid((unsigned int*) a1, (unsigned int*) a2);
                break;
                
            default:
                if (currentProcess->exceptionVector[1] != NULL) {
		            copyState((state_t*)SYSBK_OLDAREA, currentProcess->exceptionVector[0]);
		            copyState(currentProcess->exceptionVector[1], &currentProcess->p_s);
                    (currentProcess->exceptionVector[0])->pc_epc += WORD_SIZE;
	            } else {
		            terminateProcess(NULL);
                }
                break;
        }

        if (currentProcess != NULL) {
            currentProcess->time_kernel += getTODLO() - process_TOD;
        }
        process_TOD = getTODLO();

        if (currentProcess != NULL) {
            LDST(&currentProcess->p_s);
        } else {
            schedule();
        }
    } else {
        PANIC();
    }
}

void trapHandler() {
	if (currentProcess->exceptionVector[5] != NULL) {
		copyState((state_t*)PGMTRAP_OLDAREA, currentProcess->exceptionVector[4]);
		copyState(currentProcess->exceptionVector[5], &currentProcess->p_s);

		LDST(&currentProcess->p_s); 
	} else {
		terminateProcess(NULL);
        schedule();
    }
}

void tlbHandler() {
    if (currentProcess->exceptionVector[3] != NULL) {
		copyState((state_t*)TLB_OLDAREA, currentProcess->exceptionVector[2]);
		copyState(currentProcess->exceptionVector[3], &currentProcess->p_s);

		LDST(&currentProcess->p_s); 
	} else {
		terminateProcess(NULL);
    }
}