#include <umps/libumps.h>
#include <umps/arch.h>

#include "interrupt.h"
#include "pcb/pcb.h"
#include "asl/asl.h"
#include "tests/p2/main.h"
#include "scheduler/scheduler.h"
#include "utils/const_rikaya.h"
#include "utils/utils.h"
#include "syscall/syscall.h"


HIDDEN void pauseCurrentProcess() {
	if (currentProcess != NULL) {
		insertProcQ(&readyQueue, currentProcess);
		currentProcess->time_kernel += getTODLO() - process_TOD;
		currentProcess = NULL;	
	}
}

HIDDEN void cpuTimerHandler() {
	setTIMER(SCHED_TIME_SLICE);
}

HIDDEN void timerHandler() {
	while (semPseudoClock < 0) {
		verhogen(&semPseudoClock);
	}

	SET_IT(SCHED_PSEUDO_CLOCK);
}

HIDDEN void interruptVerhogen(int *sem, int statusRegister, memaddr* kernelStatusDev) {
    pcb_t* first = removeBlocked(sem);
    (*sem)++;

    if (first != NULL) {
        first-> p_s.reg_v0 = statusRegister;
        first->p_semkey = NULL;
        insertProcQ(&readyQueue, first);
    } else {
        (*kernelStatusDev) = statusRegister;
    }
}

HIDDEN void ackAndVerhogen(int intLine, int device, int statusReg, memaddr *commandReg) {
	memaddr *semDev 		 = getSemDev(intLine, device);
	memaddr *kernelStatusDev = getKernelStatusDev(intLine, device);

	(*commandReg) = DEV_C_ACK;

	interruptVerhogen((int *) semDev, statusReg, kernelStatusDev);
}

HIDDEN int getHighestPriorityDev(memaddr* line) {
	int activeBit = 0x00000001;

	for (int i = 0; i < 8; i++) {
		if (((*line)&activeBit) == activeBit) {
			return i;
		}
		activeBit = activeBit << 1;
	}

	return -1;
}

HIDDEN void genericDevHandler(int interruptLineNum) {
	memaddr *intLine = (memaddr*) CDEV_BITMAP_ADDR(interruptLineNum);
	int device = getHighestPriorityDev(intLine);

	memaddr *commandReg = (memaddr*) (DEV_REG_ADDR(interruptLineNum, device) + DEV_REG_LEN);
	memaddr *statusReg 	= (memaddr*) (DEV_REG_ADDR(interruptLineNum, device));

	ackAndVerhogen(interruptLineNum, device, (*statusReg), commandReg);
}

HIDDEN void terminalHandler() {
	memaddr *intLine = (memaddr*) CDEV_BITMAP_ADDR(IL_TERMINAL);

	int device = getHighestPriorityDev(intLine);

	memaddr  terminalRegister = (memaddr)  (DEV_REG_ADDR(IL_TERMINAL, device));
	memaddr* statusRegRead    = (memaddr*) (terminalRegister + RECSTATUS * DEV_REG_LEN);
	memaddr* commandRegRead   = (memaddr*) (terminalRegister + RECCOMMAND * DEV_REG_LEN);
	memaddr* statusRegWrite	  = (memaddr*) (terminalRegister + TRANSTATUS * DEV_REG_LEN);
	memaddr* commandRegWrite  = (memaddr*) (terminalRegister + TRANCOMMAND * DEV_REG_LEN);

	if (((*statusRegWrite) & 0x0F) == DEV_TTRS_S_CHARTRSM) {
		ackAndVerhogen((IL_TERMINAL/* + 1*/), device, ((*statusRegWrite)), commandRegWrite);
	} else if (((*statusRegRead) & 0x0F) == DEV_TRCV_S_CHARRECV) {
		ackAndVerhogen(IL_TERMINAL, device, ((*statusRegRead)), commandRegRead);
	}
}

/**
  * @brief Hanldes all interrupts, restores current process state and calls scheduler.
  * @return void.
 */
void intHandler() {
	if (currentProcess != NULL) {
		copyState((state_t*) INT_OLDAREA, &(currentProcess->p_s));
		currentProcess->time_user += getTODLO() - process_TOD;
	}
	process_TOD = getTODLO();

	int cause = getCAUSE();

	if (CAUSE_IP_GET(cause, INT_T_SLICE)) {
		cpuTimerHandler();
	} else if (CAUSE_IP_GET(cause, INT_TIMER)) {
		timerHandler();
	} else if (CAUSE_IP_GET(cause, INT_DISK)) {
		genericDevHandler(INT_DISK);
	} else if (CAUSE_IP_GET(cause, INT_TAPE)) {
		genericDevHandler(INT_TAPE);
	} else if (CAUSE_IP_GET(cause, INT_UNUSED)) {
		genericDevHandler(INT_UNUSED);
	} else if (CAUSE_IP_GET(cause, INT_PRINTER)) {
		genericDevHandler(INT_PRINTER);
	} else if (CAUSE_IP_GET(cause, INT_TERMINAL)) {
		terminalHandler();
	}

	pauseCurrentProcess();

	schedule();
}