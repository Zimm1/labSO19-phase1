#include <umps/libumps.h>

#include "scheduler.h"
#include "pcb/pcb.h"
#include "tests/p1.5/main.h"
#include "tests/p1.5/p1.5test_rikaya_v0.h"
#include "utils/const.h"
#include "utils/utils.h"

HIDDEN void setNextTimer() {
	setTIMER(TIME_SLICE);
}

/**
  * @brief increments the priority of all processes in the ready queue in order to avoid starvation.
  * @return void.
 */
HIDDEN void aging() {
	pcb_t* item;
	list_for_each_entry(item, &readyQueue, p_next) {
		item->priority++;
	}
}

/**
  * @brief Removes from the queue of ready processes the PCB with highest priority and load his state in the CPU.
  * @return void.
 */
void schedule() {
	setNextTimer();
	
	if (!emptyProcQ(&readyQueue)) {
		currentProcess = removeProcQ(&readyQueue);
		/* Reset the priority of removed process to its original priority. */
		currentProcess->priority = currentProcess->original_priority;
		log_process_order(currentProcess->original_priority);

		aging();

		LDST(&(currentProcess->p_s));
	} else {
		HALT();
	}
}