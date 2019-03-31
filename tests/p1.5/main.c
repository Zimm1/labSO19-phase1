#ifndef MAIN_H
#define MAIN_H

#include "pcb/pcb.h"

struct list_head readyQueue;
pcb_t *currentProcess;

#include <umps/arch.h>

#include "p1.5test_rikaya_v0.c"

#include "scheduler.c"
#include "syscall.c"
#include "interrupts.c"

void initStatus(state_t* state) {
    state->status = (((state->status | INT_MASK_ON_OR) & VM_OFF_AND) & KERNEL_ON_AND) | TIMER_ON_OR;
}

void initArea(memaddr area, memaddr handler){
    state_t *newArea = (state_t*) area;
    /* Setta pc alla funzione che gestirà l'eccezione */
    newArea->pc_epc = handler;
    /* Setta sp a RAMTOP */
    newArea->reg_sp = RAMTOP;
    newArea->reg_t9 = RAMTOP;
    /* Setta il registro di Stato per mascherare tutti gli interrupt e si mette in kernel-mode. */
    initStatus(newArea);
}

void initProccesses() {
  for (int i = 1; i <= 3; i++) {
    pcb_t *process = allocPcb();
    if (process == NULL) {
      PANIC();
    }

    /* Abilita gli interrupt, il Local Timer e la kernel-mode */
    initStatus(&(process->p_s));
    /* Assegna ad SP il valore RAMTOP - FRAMESIZE * la priorità i */
    process->p_s.reg_sp = RAMTOP - FRAMESIZE * i;
    /* Assegno la priorità e l'original priority uguale a numero del test */
    process->original_priority = process->priority = i;

    /* Assegno i vari test (1, 2, 3) a seconda del valore di i */
    switch (i) {
      case 1:
        process->p_s.pc_epc = (memaddr) test1;
      break;

      case 2:
        process->p_s.pc_epc = (memaddr) test2;
      break;

      case 3:
        process->p_s.pc_epc = (memaddr) test3;
      break;
    }

    insertProcQ(&readyQueue, process);
  }
}

int main() {
  /* Popola le 4 nuove Aree nella ROM */
  initArea(SYSBP_NEW_AREA,  (memaddr) sysBpHandler);
  initArea(TRAP_NEW_AREA,   (memaddr) 0);
  initArea(TLB_NEW_AREA,    (memaddr) 0);
  initArea(INT_NEW_AREA,    (memaddr) intHandler);

  /* Inizializza la stuttura dati dei pcbs */
  initPcbs();

  /* Inizializza la lista dei processi (readyQueue) */
  mkEmptyProcQ(&readyQueue);

  /* Alloco i 3 processi e li aggiungo alla readyQueue */
  initProccesses();

  schedule();

  return -1;
}

#endif
