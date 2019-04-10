#include <umps/libumps.h>

#include "scheduler.h"
#include "pcb/pcb.h"
#include "tests/p1.5/main.h"
#include "tests/p1.5/p1.5test_rikaya_v0.h"
#include "utils/const.h"
#include "utils/utils.h"

void setNextTimer() {
	setTIMER(TIME_SLICE);
}

void aging() {
	pcb_t* item;
	list_for_each_entry(item, &readyQueue, p_next) {
		item->priority++;
	}
}

void schedule() {
	if (currentProcess != NULL) {
		copyState((state_t*) INT_OLD_AREA, &(currentProcess->p_s));
		insertProcQ(&readyQueue, currentProcess);
	}

	setNextTimer();
	
	if (!emptyProcQ(&readyQueue)) {
		currentProcess = removeProcQ(&readyQueue);
		currentProcess->priority = currentProcess->original_priority;
		log_process_order(currentProcess->original_priority);

		aging();

		LDST(&(currentProcess->p_s));
	} else {
		HALT();
	}
}