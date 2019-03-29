
#include <libuarm.h>
#include <uARMconst.h>
#include <uARMtypes.h>
#include <arch.h>

#include "include/base.h"
#include "include/const.h"
#include "include/types10.h"

#include "include/pcb.h"
#include "include/asl.h"
    
#include "include/initial.h"
#include "include/scheduler.h"
#include "include/exceptions.h"
#include "include/syscall.h"

void terminateProcess(pcb_t *pcb){
    if(pcb!=NULL){
        while(!emptyChild(pcb)){
            terminateProcess(pcb->p_child);
        }
        outChild(pcb);
        if(currentProcess == pcb){
            currentProcess = NULL;
        }
        outProcQ(&readyQueue, pcb);
        freePcb(pcb);
        processCount--;
    }
}

void saveStateIn(state_t *from, state_t *to){
        to->a1                  = from->a1;
        to->a2                  = from->a2;
        to->a3                  = from->a3;
        to->a4                  = from->a4;
        to->v1                  = from->v1;
        to->v2                  = from->v2;
        to->v3                  = from->v3;
        to->v4                  = from->v4;
        to->v5                  = from->v5;
        to->v6                  = from->v6;
        to->sl                  = from->sl;
        to->fp                  = from->fp;
        to->ip                  = from->ip;
        to->sp                  = from->sp;
        to->lr                  = from->lr;
        to->pc                  = from->pc;
        to->cpsr                = from->cpsr;
        to->CP15_Control        = from->CP15_Control;
        to->CP15_EntryHi        = from->CP15_EntryHi;
        to->CP15_Cause          = from->CP15_Cause;
        to->TOD_Hi              = from->TOD_Hi;
        to->TOD_Low             = from->TOD_Low;
}

void sysBpHandler(){
    saveStateIn(sysbp_old, &currentProcess->p_s);
    unsigned int cause = CAUSE_EXCCODE_GET(sysbp_old->CP15_Cause);
    unsigned int a0 = (*sysbp_old).a1;
    unsigned int a1 = (*sysbp_old).a2;
    unsigned int a2 = (*sysbp_old).a3;
    unsigned int a3 = (*sysbp_old).a4;
    if(cause==EXC_SYSCALL){
        if( (currentProcess->p_s.cpsr & STATUS_SYS_MODE) == STATUS_SYS_MODE){
            switch(a0){

                case TERMINATEPROCESS:
                    terminateProcess(currentProcess);
                    break;     

            }
