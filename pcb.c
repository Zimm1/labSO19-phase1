#include "pcb.h"
#include "const.h"

HIDDEN pcb_t pcbfree_h;
HIDDEN pcb_t pcbFree_table[MAXPROC];

void initPcbs(void) {
	mkEmptyProcQ(&pcbfree_h.p_next);

	for (int i = 0; i < MAXPROC; ++i) {
		freePcb(&pcbFree_table[i]);
	}
}

void freePcb(pcb_t *p) {
	insertProcQ(&pcbfree_h.p_next, p);
}

pcb_t *allocPcb(void) {
	pcb_t *p = removeProcQ(&pcbfree_h.p_next);

	if (p != NULL) {
		INIT_LIST_HEAD(&p->p_next);
		
		INIT_LIST_HEAD(&p->p_child);
		INIT_LIST_HEAD(&p->p_sib);

		p->p_parent = NULL;
		p->p_semkey = NULL;

		p->p_s.entry_hi = 0;
		p->p_s.cause = 0;
		p->p_s.status = 0;
		p->p_s.pc_epc = 0;
		p->p_s.hi = 0;
		p->p_s.lo = 0;

		p->priority = 0;
	}

	return p;
}


void mkEmptyProcQ(struct list_head *head) {
	INIT_LIST_HEAD(head);
}

int emptyProcQ(struct list_head *head) {
	return list_empty(head);
}

void insertProcQ(struct list_head *head, pcb_t *p) {
	pcb_t *currentP;
	list_for_each_entry(currentP, head, p_next) {
    	if (p->priority > currentP->priority) {
    		list_add(&p->p_next, currentP->p_next.prev);
    		return;
    	}
	}

	list_add_tail(&p->p_next, head);
}

pcb_t *headProcQ(struct list_head *head) {
	if (emptyProcQ(head)) {
		return NULL;
	}

	return container_of(head->next, pcb_t, p_next);
}


pcb_t *removeProcQ(struct list_head *head) {
	pcb_t *p = headProcQ(head);
	list_del(head->next);
	return p;
}

pcb_t *outProcQ(struct list_head *head, pcb_t *p) {
	pcb_t *currentP;
	list_for_each_entry(currentP, head, p_next) {
    	if (p == currentP) {
    		list_del(&p->p_next);
    		return p;
    	}
	}
	
	return NULL;
}


int emptyChild(pcb_t *this) {
	return emptyProcQ(&this->p_child);
}

void insertChild(pcb_t *prnt, pcb_t *p) {
	list_add_tail(&p->p_sib, &prnt->p_child);
	p->p_parent = prnt;
}

pcb_t *removeChild(pcb_t *p) {
	if (emptyChild(p)) {
		return NULL;
	}

	pcb_t* currentP = container_of(p->p_child.next, pcb_t, p_sib);
	list_del(&currentP->p_sib);
	return currentP;
}

pcb_t *outChild(pcb_t *p) {
	if (p->p_parent == NULL) {
		return NULL;
	}

	list_del(&p->p_sib);
    return p;
}
