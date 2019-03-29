#include <umps/arch.h>
#include <umps/libumps.h>

#include "pcb/pcb.h"

unsigned int time_slice = 3;

void setNextTimer(){
	setTIMER(time_slice);
}

void aging(){
	pcb_t *item;
	list_for_each_entry(item, ready_queue, p_next) {
		item->priority = item->priority + 1;
	}
}

void schedule(){

	setNextTimer();

	if(!emptyProcQ(ready_queue)){
		pcb_t *current_process = (pcb_t*) removeProcQ(ready_queue);

		current_process->priority = current_process->original_priority;

		aging();

		log_process_order(current_process->original_priority);

		LDST(&(current_process->p_s));
	} else {
		WAIT();
	}

}
