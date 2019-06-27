#ifndef SCHEDULER_H
#define SCHEDULER_H

unsigned int slice_TOD;
unsigned int clock_TOD;
unsigned int process_TOD;

unsigned int MUTEX_SCHEDULER;

int isTimer(unsigned int TIMER_TYPE);
void schedule();

#endif