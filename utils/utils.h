#ifndef UTILS_H
#define UTILS_H

#include "utils/types_rikaya.h"

void copyState(state_t* src, state_t* dest) {
	dest->entry_hi = src->entry_hi;
	dest->cause = src->cause;
	dest->status = src->status;
	dest->pc_epc = src->pc_epc;

	for(int i = 0; i < STATE_GPR_LEN + 2; ++i){
		dest->gpr[i] = src->gpr[i];
	}
}

#endif