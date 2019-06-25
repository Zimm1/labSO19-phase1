#include <umps/arch.h>
#include <umps/libumps.h>

#include "main.h"
#include "p2test_rikaya_v0.1.h"
#include "pcb/pcb.h"
#include "asl/asl.h"
#include "scheduler/scheduler.h"
#include "syscall/syscall.h"
#include "interrupt/interrupt.h"
#include "utils/const_rikaya.h"

struct list_head readyQueue; /* Ready process list */
pcb_t *currentProcess; /* Currently running process */

int semCpuTimer;
int semDev[N_EXT_IL + 1][N_DEV_PER_IL];
int statusDev[N_EXT_IL + 1][N_DEV_PER_IL];
int semPseudoClock;

memaddr* getSemDev(int line, int dev) {
    if (3 <= line && line <= 8 && 0 <= dev && dev <= 7) {
        return (memaddr *) &semDev[line - DEV_IL_START][(N_DEV_PER_IL-1) -dev];
    }

    return NULL;
}

memaddr* getKernelStatusDev(int line, int dev) {
    if(3 <= line  && line <= 8 && 0 <= dev && dev <= 7) {
        return (memaddr *) &statusDev[line - DEV_IL_START][(N_DEV_PER_IL-1) -dev];
    }
    
    return NULL;
}

/**
  * @brief Disables interrupts and vm, enables kernel mode and local timer
  * @return void.
 */
HIDDEN void initAreaStatus(state_t* state) {
    state->status = TIMER_ON_OR;
}

/**
  * @brief Disables vm, enables all interrupts, kernel mode and local timer
  * @return void.
 */
HIDDEN void initProcessStatus(state_t* state) {
    state->status = INT_MASK_ON_OR | TIMER_ON_OR;
}

/**
  * @brief Sets handler, stack pointer and status for an area.
  * @return void.
 */
HIDDEN void initArea(memaddr area, memaddr handler) {
    state_t *newArea = (state_t*) area;

    newArea->pc_epc = handler;
    newArea->reg_sp = RAMTOP;
    initAreaStatus(newArea);
}

/**
  * @brief Creates a process and sets its priority, stack pointer and status.
  * @return void.
 */
HIDDEN void initProcess(int index, memaddr address) {
    pcb_t *process = allocPcb();
    if (process == NULL) {
        PANIC();
    }

    process->p_s.pc_epc = address;
    process->original_priority = process->priority = index;
    process->p_s.reg_sp = RAMTOP - FRAME_SIZE * index;
    initProcessStatus(&(process->p_s));

    insertProcQ(&readyQueue, process);
}

state_t* trap_old = (state_t*) PGMTRAP_OLDAREA;
unsigned int vmc;

void trapHandler() {
    vmc = CAUSE_EXCCODE_GET(trap_old->cause);
    PANIC();
}

void tlbHandler() {
    PANIC();
}

/**
  * @brief Initializes umps ROM areas.
  * @return void.
 */
HIDDEN void initAreas() {
    initArea(SYSBK_NEWAREA,  (memaddr) sysBpHandler);
    initArea(INT_NEWAREA,    (memaddr) intHandler);
    initArea(PGMTRAP_NEWAREA, (memaddr) trapHandler);
    initArea(TLB_NEWAREA, (memaddr) tlbHandler);
}

/**
  * @brief Initializes the pcbs, process queue and needed processes.
  * @return void.
 */
HIDDEN void initProccesses() {
    initPcbs();
    mkEmptyProcQ(&readyQueue);

    initProcess(1, (memaddr) test);
}

/**
  * @brief Initializes the semaphores.
  * @return void.
 */
HIDDEN void initAsl() {
    initASL();

    for (int i = 0; i < N_EXT_IL + 1; ++i) {
        for (int j = 0; j < N_DEV_PER_IL; ++j) {
            semDev[i][j] = 0;
        }
    }
}

int main() {
    initAreas();
    initProccesses();
    initAsl();
    
    schedule();

    return -1;
}
