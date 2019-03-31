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

    /* Imposta il registro sp a RAM_TOP */
	  newArea->reg_sp = RAM_TOP;

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

    /* Interrupt attivati e smascherati, Memoria Virtuale spenta, Kernel-Mode attivo */
	  Process->p_state.status = (Process->p_state.status | STATUS_IEp | STATUS_INT_UNMASKED | STATUS_KUc) & ~STATUS_VMp;

	  /* Il registro SP viene inizializzato a RAMTOP-FRAMESIZE */
	  init->p_state.reg_sp = (RAMTOP - FRAME_SIZE) * i;

    /* Assegno la priorità e l'original priority uguale a numero del test */
    Process->original_priority = Process->priority = i;

    /* Assegno i vari test (1, 2, 3) a seconda del valore di i */
    switch (i) {
      case 1:
        Process->p_state.pc_epc = Process->p_state.reg_t9 = (memaddr) test1;
      break;

      case 2:
        Process->p_state.pc_epc = Process->p_state.reg_t9 = (memaddr) test2;
      break;

      case 3:
        Process->p_state.pc_epc = Process->p_state.reg_t9 = (memaddr) test3;      
      break;
    }


    insertProcQ(&readyQueue, Process);
  }

  schedule();
}
