#include <umps/arch.h>
#include <umps/libumps.h>

#include "syscall.h"
#include "scheduler.h"
#include "tests/p1.5/main.h"
#include "tests/p1.5/p1.5test_rikaya_v0.h"
#include "pcb/pcb.h"
#include "utils/types_rikaya.h"
#include "utils/const.h"

int getCauseExcCode() {
    return (((state_t*) SYSBP_OLD_AREA)->cause & 0x00003C) >> 2;
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

    schedule();
}

void sysBpHandler(){
    state_t *sysbp_old = (state_t*) SYSBP_OLD_AREA;

    unsigned int cause = getCauseExcCode();
    unsigned int a0 = (*sysbp_old).reg_a0;
    /*unsigned int a1 = (*sysbp_old).reg_a1;
    unsigned int a2 = (*sysbp_old).reg_a2;
    unsigned int a3 = (*sysbp_old).reg_a3;*/

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
