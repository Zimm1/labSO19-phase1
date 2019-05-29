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
  * @brief (SYS3) Terminates a process and all of its children.
  * @param pcb : Process to terminate.
  * @return void.
 */
HIDDEN void terminateProcess(pcb_t* pcb) {
    if (pcb != NULL) {
        while(!emptyChild(pcb)) {
            terminateProcess(removeChild(pcb));
        }
        outChild(pcb);
        if (currentProcess == pcb) {
            currentProcess = NULL;
        }

        outProcQ(&readyQueue, pcb);
        freePcb(pcb);
    }

    schedule();
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

// SYS4
void passeren(int *sem) {
    (*sem)--;

    if ((*sem)<0) {
        currentProcess->p_semkey = sem;
        currentProcess->cpu_time += getTODLO() - process_TOD;

        insertBlocked(sem, currentProcess);
        currentProcess = NULL;
    }
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

    if (cause == EXC_SYS) {
        switch(a0) {
            case TERMINATEPROCESS:
                terminateProcess(currentProcess);
                break;

            case VERHOGEN:
                verhogen((int *) a1);
                break;

            case PASSEREN:
                passeren((int *) a1);
                break;

            case WAITIO:
                doIO(a1, (int *) a2);
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