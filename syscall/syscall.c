#include <umps/libumps.h>

#include "syscall.h"
#include "scheduler/scheduler.h"
#include "tests/p1.5/main.h"
#include "pcb/pcb.h"
#include "utils/types_rikaya.h"
#include "utils/const.h"

/**
  * @brief Extracts the cause from the cause register
  * @param pcb : Cause register.
  * @return Extracted cause.
 */
HIDDEN int getCauseExcCode(int cause) {
    return (cause & EXC_CODE_MASK) >> EXC_CODE_SHIFT;
}

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

/**
  * @brief System calls and breakpoints handler, checks the cause and calls the right sub-handler
  * @return void.
 */
void sysBpHandler() {
    state_t* sysbp_old = (state_t*) SYSBP_OLD_AREA;

    unsigned int cause = getCauseExcCode(sysbp_old->cause);
    unsigned int a0 = sysbp_old->reg_a0;

    if (cause == EXC_SYS) {
        switch(a0) {
            case SYS_TERMINATE_PROCESS:
                terminateProcess(currentProcess);
                break;     

            default:
                PANIC();
                break;
        }
    }
}
