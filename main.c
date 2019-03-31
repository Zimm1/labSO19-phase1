#include "tests/p1.5/p1.5test_rikaya_v0.c"
#include "pcb/pcb.h"

#include "scheduler.c"
#include "exceptions.c"
#include "interrupts.c"



void initArea(memaddr area, memaddr handler){
    state_t *newArea = (state_t*) area;
    /* Memorizza il contenuto attuale del processore in newArea */
    STST(newArea);

    /* Imposta il PC all'indirizzo del gestore delle eccezioni */
	  newArea->pc_epc = newArea->reg_t9 = handler;

    /* Imposta il registro sp a RAMTOP */
	  newArea->reg_sp = RAMTOP;

	  /* Interrupt mascherati, Memoria Virtuale spenta, Kernel Mode attivo */
	  newArea->status = (newArea->status | STATUS_KUc) & ~STATUS_INT_UNMASKED & ~STATUS_VMp;
}


int main() {

  /* Popola le 4 nuove Aree nella ROM */
  initArea(INT_NEWAREA,       (memaddr) intHandler);
  initArea(TLB_NEWAREA,       (memaddr) tlbHandler);
  initArea(PGMTRAP_NEWAREA,   (memaddr) pgmHandler);
  initArea(SYSBK_NEWAREA,     (memaddr) sysBpHandler);


  /* Inizializza la stuttura dati dei pcbs */
  initPcbs();

  /* Inizializza la lista dei processi (readyQueue) */
  LIST_HEAD(readyQueue);

  /* Alloco i 3 processi e li aggiungo alla readyQueue */
  for (int i = 1; i <= 3; i++) {
    pcb_t *Process = allocPcb();
    if (Process == NULL) {
      PANIC();
    }

    /* Abilita gli interrupt, il Local Timer e la kernel-mode */
    Process->p_s.cpsr = STATUS_ALL_INT_ENABLE(Process->p_s.cpsr) | STATUS_SYS_MODE;
    /* Disabilita la memoria virtuale */
    Process->p_s.CP15_Control = (Process->p_s.CP15_Control) & ~(ENABLE_VM);
    /* Assegna ad SP il valore (RAMTOP - FRAMESIZE) * la priorità i */
    Process->p_s.sp = (RAM_TOP - FRAME_SIZE) * i;
    /* Assegno la priorità e l'original priority uguale a numero del test */
    Process->original_priority = Process->priority = i;

    /* Assegno i vari test (1, 2, 3) a seconda del valore di i */
    switch (i) {
      case 1:
        Process->p_s.pc = (memaddr) test1;
      break;

      case 2:
        Process->p_s.pc = (memaddr) test2;
      break;

      case 3:
        Process->p_s.pc = (memaddr) test3;
      break;
    }


    insertProcQ(&readyQueue, Process);
  }

  schedule();
}
