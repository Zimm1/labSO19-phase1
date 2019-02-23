#include "asl.h"
#include "pcb.h"
#include "const.h"

HIDDEN struct list_head semdfree_h;
HIDDEN struct list_head semd_h;
HIDDEN semd_t semd_table[MAXPROC];

/**
  * @brief searches a semaphore descriptor in the ASL (active semaphores list).
  *	@param key : pointer to semaphore descriptor searched.  
  * @return returns the pointer to SEMD (semaphore descriptor) if it exists in the ASL (active semaphores list), otherwise NULL.
 */
semd_t* getSemd(int *key) {
	semd_t *iter;
	list_for_each_entry(iter, &semd_h, s_next) {
		if (iter->s_key == key) {
			return iter;
		}
	}

	return NULL;
}

/**
  * @brief creates the list of unused SEMD (semaphores descriptors).
  * @return void.
 */
void initASL() {
	INIT_LIST_HEAD(&semdfree_h);
	INIT_LIST_HEAD(&semd_h);

	int i;

	for (i = 0; i < MAXPROC; i++) {
		semd_t *s = &semd_table[i];
		list_add(&s->s_next, &semdfree_h);
	}
}


/**
  * @brief inserts the PCB (pointed by p) in the queue of processes blocked associated to SEMD (with value key).
  * @param key : pointer to SEMD searched.
  * @param p : pointer to PCB searched in the queue of PCB contained in the SEMD.
  * @return returns TRUE if it's impossible to allocate a new SEMD because the list of unused semaphores descriptors is empty, otherwise FALSE.
 */
int insertBlocked(int *key, pcb_t* p) {
	semd_t *s = getSemd(key);

	if (s != NULL) {
		insertProcQ(&s->s_procQ, p);
		p->p_semkey = key;
		return FALSE;
	}

	if (list_empty(&semdfree_h)) {
		return TRUE;
	}
	
	s = container_of(semdfree_h.next, semd_t, s_next);
	list_del(semdfree_h.next);

	INIT_LIST_HEAD(&s->s_next);
	INIT_LIST_HEAD(&s->s_procQ);
	s->s_key = key;

	insertProcQ(&s->s_procQ, p);

	semd_t *list_s;
	list_for_each_entry(list_s, &semd_h, s_next) {
		if (s->s_key > list_s->s_key) {
			list_add(&s->s_next, &list_s->s_next);
			p->p_semkey = key;
			return FALSE;
		}
	}

	list_add(&s->s_next, &semd_h);
	p->p_semkey = key;
	return FALSE;
}

/**
  * @brief removes the first PCB from the queue of processes blocked associated to SEMD (with value key).
  * @param key : pointer to SEMD searched.
  * @return returns NULL if the searched semaphore descriptor doesn't exist in the ASL (active semaphores list), otherwise the removed item.
 */
pcb_t* removeBlocked(int *key) {
	semd_t *s = getSemd(key);

	if (s == NULL) {
		return NULL;
	}

	pcb_t *p = container_of(s->s_procQ.next, pcb_t, p_next);
	p->p_semkey = NULL;

	list_del(s->s_procQ.next);

	if (emptyProcQ(&s->s_procQ)) {
		list_del(&s->s_next);
		list_add(&s->s_next, &semdfree_h);
	}
	return p;
}

/**
  * @brief removes the given PCB from the queue of PCB in the SEMD where it is blocked.
  * @param p : pointer to  PCB to remove.
  * @return returns NULL if PCB doesn't exist in the PCB queue of SEMD searched, otherwise p.
 */
pcb_t* outBlocked(pcb_t *p) {
	semd_t *s = getSemd(p->p_semkey);

	if (s == NULL) {
		return NULL;
	}

	pcb_t *tmp = outProcQ(&s->s_procQ, p);
	tmp->p_semkey = NULL;

	if (emptyProcQ(&s->s_procQ)) {
		list_del(&s->s_next);
		list_add(&s->s_next, &semdfree_h);
	}

	return tmp;
}

/**
  * @brief returns (without removing) the head of the queue of PCB associated to SEMD.
  * @param key : pointer to SEMD searched
  * @return returns NULL if the SEMD isn't in the ASL or the queue of its processes is empty, otherwise the head of the queue.
 */
pcb_t* headBlocked(int *key) {
	semd_t *s = getSemd(key);

	if (s == NULL || emptyProcQ(&s->s_next)) {
		return NULL;
	}

	return container_of(s->s_procQ.next, pcb_t, p_next);
}

/**
  * @brief removes the process p from the queue of the SEMD (semaphores descriptors) where it is blocked. 
  			Furthermore, removes all processes that have as a forefather p.
  * @param p : pointer to PCB to remove.
  * @return void.
 */
void outChildBlocked(pcb_t *p) {
	outBlocked(p);

	pcb_t *child;
	list_for_each_entry(child, &p->p_child, p_sib) {
		outChildBlocked(child);
	}
}
