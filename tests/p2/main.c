#include <umps/arch.h>
#include <umps/libumps.h>

#include "main.h"
#include "p1.5test_rikaya_v0.h"
#include "pcb/pcb.h"
#include "asl/asl.h"
#include "scheduler/scheduler.h"
#include "syscall/syscall.h"
#include "interrupt/interrupt.h"
#include "utils/const.h"

struct list_head readyQueue; /* Ready process list */
pcb_t *currentProcess; /* Currently running process */

/**
  * @brief Disables interrupts and vm, enables kernel mode and local timer
  * @return void.
 */
HIDDEN void initAreaStatus(state_t* state) {
  state->status = TIMER_ON_OR;
}

/**
  * @brief Disables all interrupts (except local timer) and vm, enables kernel mode and local timer
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
  process->p_s.reg_sp = RAMTOP - FRAMESIZE * index;
  initProcessStatus(&(process->p_s));

  insertProcQ(&readyQueue, process);
}

/**
  * @brief Initializes umps ROM areas.
  * @return void.
 */
HIDDEN void initAreas() {
  initArea(SYSBP_NEW_AREA,  (memaddr) sysBpHandler);
  initArea(INT_NEW_AREA,    (memaddr) intHandler);
}

/**
  * @brief Initializes the pcbs, process queue and needed processes.
  * @return void.
 */
HIDDEN void initProccesses() {
  initPcbs();
  mkEmptyProcQ(&readyQueue);

  initProcess(1, (memaddr) test1);
  initProcess(2, (memaddr) test2);
  initProcess(3, (memaddr) test3);
}

/**
  * @brief Initializes the semaphores.
  * @return void.
 */
HIDDEN void initAsl() {
  initASL();
}

int main() {
  initAreas();
  initProccesses();
  initAsl();
  
  schedule();

  return -1;
}
