#include "interrupt.h"
#include "pcb/pcb.h"
#include "tests/p1.5/main.h"
#include "scheduler/scheduler.h"
#include "utils/const.h"
#include "utils/utils.h"


void intHandler() {
	if (currentProcess != NULL) {
		copyState((state_t*) INT_OLD_AREA, &(currentProcess->p_s));
		insertProcQ(&readyQueue, currentProcess);
	}

	schedule();
}