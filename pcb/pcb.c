#include "pcb.h"
#include "utils/const.h"

HIDDEN pcb_t pcbfree_h;
HIDDEN pcb_t pcbFree_table[MAXPROC];

/**
  * @brief Initializes the free pcb list.
  * @return void.
 */
void initPcbs(void) {
	mkEmptyProcQ(&pcbfree_h.p_next);

	for (int i = 0; i < MAXPROC; ++i) {
		freePcb(&pcbFree_table[i]);
	}
}

/**
  * @brief Adds a pcb in the free pcb list.
  * @param p : Pcb pointer.
  * @return void.
 */
void freePcb(pcb_t *p) {
	insertProcQ(&pcbfree_h.p_next, p);
}

/**
  * @brief Allocates a pcb and removes it from the free pcb list.
  * @return Pointer to allocated pcb, NULL if the free pcb list is empty
 */
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
		p->original_priority = 0;
	}

	return p;
}


/**
  * @brief Initializes an empty pcb queue.
  * @param emptylist : Queue pointer.
  * @return void.
 */
void mkEmptyProcQ(struct list_head *head) {
	INIT_LIST_HEAD(head);
}

/**
  * @brief Checks whether a pcb queue is empty.
  * @param head : Queue empty.
  * @return True (1) if the queue is empty, false (0) otherwise.
 */
int emptyProcQ(struct list_head *head) {
	return list_empty(head);
}

/**
  * @brief Adds a pcb to a pcb queue.
  * @param head : Queue pointer.
  * @param p : Pcb pointer.
  * @return void.
 */
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

/**
  * @brief Gets the first element of a pcb queue.
  * @param head : Queue pointer.
  * @return First pcb pointer in the queue, NULL if the queue is empty.
 */
pcb_t *headProcQ(struct list_head *head) {
	if (emptyProcQ(head)) {
		return NULL;
	}

	return container_of(head->next, pcb_t, p_next);
}

/**
  * @brief Removes the first pcb of a pcb queue.
  * @param head : Queue pointer.
  * @return Removed pcb pointer, NULL if the queue is empty.
 */
pcb_t *removeProcQ(struct list_head *head) {
	pcb_t *p = headProcQ(head);
	list_del(head->next);
	return p;
}


/**
  * @brief Removes a pcb from a pcb queue.
  * @param head : Queue pointer.
  * @param p : Pcb pointer.
  * @return Removed pcb pointer, NULL if the pcb is not in the queue.
 */
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


/**
  * @brief Checks whether a pcb has (not) children.
  * @param child : Pcb pointer.
  * @return True (1) if the pcb hasn't any children, false (0) otherwise.
 */
int emptyChild(pcb_t *this) {
	return emptyProcQ(&this->p_child);
}

/**
  * @brief Adds a child to a pcb.
  * @param parent : Parent pcb pointer.
  * @param child : Child pcb ponter.
  * @return void.
 */
void insertChild(pcb_t *prnt, pcb_t *p) {
	list_add_tail(&p->p_sib, &prnt->p_child);
	p->p_parent = prnt;
}

/**
  * @brief Removes the first child of a pcb
  * @param parent : Parent pcb pointer.
  * @return Removed child pcb pointer, NULL if the parent pcb hasn't any children.
 */
pcb_t *removeChild(pcb_t *p) {
	if (emptyChild(p)) {
		return NULL;
	}

	pcb_t* currentP = container_of(p->p_child.next, pcb_t, p_sib);
	list_del(&currentP->p_sib);
	return currentP;
}

/**
  * @brief Removes a pcb from its parent children.
  * @param child : Child pcb pointer.
  * @return Removed child pcb pointer, NULL if the parent pcb hasn't any children.
 */
pcb_t *outChild(pcb_t *p) {
	if (p->p_parent == NULL) {
		return NULL;
	}

	list_del(&p->p_sib);
    return p;
}
