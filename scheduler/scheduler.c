#include <umps/libumps.h>

#include "scheduler.h"
#include "pcb/pcb.h"
#include "asl/asl.h"
#include "tests/p2/main.h"
#include "utils/const_rikaya.h"
#include "utils/utils.h"

unsigned int MUTEX_SCHEDULER = 1;

int isTimer(unsigned int TIMER_TYPE) {
    int time_until_timer;

    if(TIMER_TYPE == SCHED_TIME_SLICE){
        time_until_timer= TIMER_TYPE - (getTODLO());
    } else if(TIMER_TYPE == SCHED_PSEUDO_CLOCK){
        time_until_timer= TIMER_TYPE - (getTODLO());
    }

    if(time_until_timer <= 0){
        return TRUE;
    } else { 
        return FALSE;
    }
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

void cpuIdle() {
	setSTATUS(getSTATUS() | STATUS_IEc | STATUS_INT_UNMASKED);
	setTIMER(SCHED_TIME_SLICE);
	while(1) WAIT();
}

/**
  * @brief Removes from the queue of ready processes the PCB with highest priority and load his state in the CPU.
  * @return void.
 */
void schedule() {
    lock(&MUTEX_SCHEDULER);

    currentProcess = NULL;

    setSTATUS(getSTATUS() & ~STATUS_IEc & ~STATUS_INT_UNMASKED);

    if (!emptyProcQ(&readyQueue)) {
        currentProcess = removeProcQ(&readyQueue);

        unlock(&MUTEX_SCHEDULER);

        if (currentProcess->time_start == 0) {
            currentProcess->time_start = getTODLO();
        }
        currentProcess->priority = currentProcess->original_priority;

        aging();
        setTIMER(SCHED_TIME_SLICE);
        LDST(&(currentProcess->p_s));
    } else {
        unlock(&MUTEX_SCHEDULER);

        cpuIdle();
    }
}