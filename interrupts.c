void intHandler(){
	/* prendo il contenuto del registro cause */
	int cause = getCAUSE();
	/* linea 1 */
	if(CAUSE_IP_GET(cause, IL_CPUTIMER)){
		lineOneTwoHandler(IL_CPUTIMER);
	} else
	/*  linea 2 timer */
	if (CAUSE_IP_GET(cause, IL_TIMER)){
	    timerHandler();
    }
	scheduler();
}

void timerHandler(){
	/* Controllo che a generare l'interrupt sia stato lo pseudo-clock */
	if(isTimer(SCHED_PSEUDO_CLOCK)){
		/* Eseguo una V su tutti i processi bloccati */
		while(semPseudoClock < 0){
			verhogen(&semPseudoClock);
		}
	} 
	if(isTimer(SCHED_TIME_SLICE)){
		/* Aggiorno il tempo passato dal processo sulla cpu */
		if(currentProcess != NULL){
			currentProcess->cpu_time += getTODLO() - process_TOD;
			insertProcQ(&readyQueue, currentProcess);
	    	currentProcess = NULL;
		}
	}
}