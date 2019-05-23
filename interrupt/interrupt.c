#include "interrupt.h"
#include "pcb/pcb.h"
#include "tests/p2/main.h"
#include "scheduler/scheduler.h"
#include "utils/const_rikaya.h"
#include "utils/utils.h"


/**
  * @brief Hanldes all interrupts, restores current process state and calls scheduler.
  * @return void.
 */
void intHandler() {
	if (currentProcess != NULL) {
		copyState((state_t*) INT_OLDAREA, &(currentProcess->p_s));
		insertProcQ(&readyQueue, currentProcess);
	}

	schedule();
}