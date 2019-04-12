#include "interrupt.h"
#include "pcb/pcb.h"
#include "tests/p1.5/main.h"
#include "scheduler/scheduler.h"
#include "utils/const.h"
#include "utils/utils.h"


/**
  * @brief Hanldes all interrupts, restores current process state and calls scheduler.
  * @return void.
 */
void intHandler() {
	if (currentProcess != NULL) {
		copyState((state_t*) INT_OLD_AREA, &(currentProcess->p_s));
		insertProcQ(&readyQueue, currentProcess);
	}

	schedule();
}