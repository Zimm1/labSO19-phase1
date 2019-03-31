#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <umps/arch.h>
#include <umps/cp0.h>

#include "tests/p1.5/main.c"
#include "pcb/pcb.h"
#include "scheduler.c"
#include "utils/const.h"

int intCause = 0;

int getHighestPriorityDev(memaddr* line){
	int activeBit = 0x00000001;
	int i;
	/* Usando una maschera (activeBit) ad ogni iterazione isolo i singoli
	bit dei device della linea. Quando ne trovo uno settato ne restituisco
	l'indice */
	for(i = 0; i < 8; i++){
		if(((*line)&activeBit) == activeBit){
			return i;
		}
		activeBit = activeBit << 1;
	}
	return -1;
}

void ack(memaddr *commandReg) {
	(*commandReg) = 1;
}

void timerHandler(){
	
}

void terminalHandler(){
	/* Uso la MACRO per ottenere la linea di interrupt */
	memaddr *intLine = (memaddr*) CDEV_BITMAP_ADDR(IL_TERMINAL);
	/* Ottengo il device a priorità più alta */
	int device = getHighestPriorityDev(intLine);
	/* Controllo il registro di stato del terminale per sapere se è stata effettuata una 
	  lettura o una scrittura */
	memaddr  terminalRegister = (memaddr)  (DEV_REG_ADDR(IL_TERMINAL, device));
	memaddr* statusRegRead    = (memaddr*) (terminalRegister + TERM_STATUS_READ);
	memaddr* commandRegRead   = (memaddr*) (terminalRegister + TERM_COMMAND_READ);
	memaddr* statusRegWrite	  = (memaddr*) (terminalRegister + TERM_STATUS_WRITE);
	memaddr* commandRegWrite  = (memaddr*) (terminalRegister + TERM_COMMAND_WRITE);
	/* Se è una scrittura (priorità più alta) */
	ack(commandRegWrite);
}

void intHandler(){
	/* prendo il contenuto del registro cause */
	intCause = getCAUSE();
	/* linea 1 */
	//if(cause == IL_CPUTIMER){
	//	lineOneTwoHandler(IL_CPUTIMER);
	//} else
	/*  linea 2 timer */
	switch (intCause) {
		case IL_IPI:
			PANIC();
			break;
	
		case IL_CPUTIMER:
		case IL_TIMER:
			timerHandler();
			break;
		
		case IL_TERMINAL:
			terminalHandler();
			break;

		default:
			PANIC();
			break;
	}

	if (currentProcess != NULL) {
		insertProcQ(&readyQueue, currentProcess);
		currentProcess = NULL;
	}
	
	schedule();
}

#endif