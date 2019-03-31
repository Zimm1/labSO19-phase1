#ifndef SYSCALL_H
#define SYSCALL_H

#include "tests/p1.5/main.c"
#include "pcb/pcb.h"
#include "utils/types_rikaya.h"

int getCauseExcCode(int cause) {
    return (cause & 0x7C) >> 2;
}

void terminateProcess(pcb_t *pcb){
    if(pcb!=NULL){
        while(!emptyChild(pcb)){
            terminateProcess(removeChild(pcb));
        }
        outChild(pcb);
        if(currentProcess == pcb){
            currentProcess = NULL;
        }

        outProcQ(&readyQueue, pcb);
        freePcb(pcb);
    }
}

void sysBpHandler(){
    state_t *sysbp_old = &currentProcess->p_s;

    unsigned int cause = getCauseExcCode(sysbp_old->cause);
    unsigned int a0 = (*sysbp_old).reg_a0;
    unsigned int a1 = (*sysbp_old).reg_a1;
    unsigned int a2 = (*sysbp_old).reg_a2;
    unsigned int a3 = (*sysbp_old).reg_a3;

    if(cause == EXC_SYS) {
        switch(a0){
            case SYS3:
                terminateProcess(currentProcess);
                break;     

            default:
                PANIC();
                break;
        }
    }
}

#endif
