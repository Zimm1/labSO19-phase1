#include "interrupt.h"
#include "scheduler/scheduler.h"


void intHandler() {
	schedule();
}