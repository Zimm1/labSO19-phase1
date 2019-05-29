#ifndef MAIN_H
#define MAIN_H

#include <umps/arch.h>
#include "pcb/pcb.h"

struct list_head readyQueue;
pcb_t *currentProcess;

int semCpuTimer;
int semDev[N_EXT_IL + 1][N_DEV_PER_IL];
int statusDev[N_EXT_IL + 1][N_DEV_PER_IL];
int semPseudoClock;

memaddr* getSemDev(int line, int dev);
memaddr* getKernelStatusDev(int line, int dev);

#endif
