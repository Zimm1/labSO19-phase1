#include <libuarm.h>
#include <uARMconst.h>
#include <uARMtypes.h>
#include <arch.h>

#include "pcb/pcb.h"
#include "interrupt.h"


unsigned int time_slice = 3;

pcb_t *current_process;

void setNextTimer(){
	setTimer(time_slice);
}

void aging(){
	pcb_t *currentP;
	list_for_each_entry(currentP, ready_queue, p_next) {
		currentP->priority = currentP->priority + 1;
	}
}

void schedule(){

	setNextTimer();

	if(!emptyProcQ(ready_queue)){
		current_process = (pcb_t*) removeProcQ(ready_queue);

		current_process->priority = current_process->original_priority;

		aging();

		log_process_order(current_process);

		LDST(&(current_process->p_s));
	} else {
		WAIT();
	}

}