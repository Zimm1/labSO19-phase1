#ifndef UTILS_H
#define UTILS_H

#include "utils/types_rikaya.h"

void copyState(state_t* src, state_t* dest);
pcb_t* getTUTOR(pcb_t* forefather);
int isParent(pcb_t* terminateProcess, pcb_t* currProcess);

#endif