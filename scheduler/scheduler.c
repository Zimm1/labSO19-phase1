#include <umps/libumps.h>

#include "scheduler.h"
#include "pcb/pcb.h"
#include "asl/asl.h"
#include "tests/p2/main.h"
#include "utils/const_rikaya.h"
#include "utils/utils.h"

/**
  * @brief Sets the timer of the closest event between the end of time slice or the system clock.
  * @return void.
 */
HIDDEN void setNextTimer(){
    unsigned int TODLO = getTODLO();
    int time_until_slice = SCHED_TIME_SLICE - (TODLO - slice_TOD);

    if(time_until_slice<=0){
        slice_TOD = TODLO;
        time_until_slice= SCHED_TIME_SLICE;
    }
    
    int time_until_clock = SCHED_PSEUDO_CLOCK - (TODLO - clock_TOD);
    if(time_until_clock <= 0){
        clock_TOD = TODLO;
        time_until_clock = SCHED_PSEUDO_CLOCK;
    }

    setTIMER((time_until_slice <= time_until_clock) ? time_until_slice : time_until_clock);
}

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
	setNextTimer();
	while(1) WAIT();
}

/**
  * @brief Removes from the queue of ready processes the PCB with highest priority and load his state in the CPU.
  * @return void.
 */
void schedule() {
    currentProcess = NULL;

    setSTATUS(getSTATUS() & ~STATUS_IEc & ~STATUS_INT_UNMASKED);

    if (!emptyProcQ(&readyQueue)) {
        currentProcess = removeProcQ(&readyQueue);

        if (currentProcess->time_start == 0) {
            currentProcess->time_start = getTODLO();
        }
        currentProcess->priority = currentProcess->original_priority;

        aging();
        setNextTimer();
        LDST(&(currentProcess->p_s));
    } else {
        cpuIdle();
    }
}