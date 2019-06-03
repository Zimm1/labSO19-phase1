#include "utils.h"
#include "types_rikaya.h"

/**
  * @brief Duplicates a state struct.
  * @return void.
 */
void copyState(state_t* src, state_t* dest) {
	dest->entry_hi = src->entry_hi;
	dest->cause = src->cause;
	dest->status = src->status;
	dest->pc_epc = src->pc_epc;

	for (int i = 0; i < STATE_GPR_LEN + 2; ++i) {
		dest->gpr[i] = src->gpr[i];
	}
}

/**
  * @brief Returns the TUTOR for the children of killed process.
 */
pcb_t* getTUTOR(pcb_t* forefather){
	if(forefather->p_parent == NULL || forefather->tutor){
		return forefather;  
	}

	return getTUTOR(forefather->p_parent);
}

/**
  * @brief Checks if the process to terminate comes from currProcess.
 */
int isParent(pcb_t* terminateProcess, pcb_t* currProcess){
	if(currProcess != NULL){
		pcb_t* itr;
		int found;
		list_for_each_entry(itr, &currProcess->p_child, p_sib){
			if(itr == terminateProcess){
				return TRUE;
			} else {
				found = isParent(terminateProcess, itr);
			}
		};
		return found;
	} else {
		return FALSE;
	}
}