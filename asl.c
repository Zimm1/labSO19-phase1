#include "asl.h"
#include "pcb.h"
#include "const.h"

HIDDEN struct list_head semdfree_h;
HIDDEN struct list_head semd_h;
HIDDEN semd_t semd_table[MAXPROC];

semd_t* getSemd(int *key) {
	semd_t *iter;
	list_for_each_entry(iter, &semd_h, s_next) {
		if (iter->s_key == key) {
			return iter;
		}
	}

	return NULL;
}

void initASL() {
	INIT_LIST_HEAD(&semdfree_h);
	INIT_LIST_HEAD(&semd_h);

	int i;

	for (i = 0; i < MAXPROC; i++) {
		semd_t *s = &semd_table[i];
		list_add(&s->s_next, &semdfree_h);
	}
}


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

pcb_t* headBlocked(int *key) {
	semd_t *s = getSemd(key);

	if (s == NULL || emptyProcQ(&s->s_next)) {
		return NULL;
	}

	return container_of(s->s_procQ.next, pcb_t, p_next);
}

void outChildBlocked(pcb_t *p) {
	outBlocked(p);

	pcb_t *child;
	list_for_each_entry(child, &p->p_child, p_sib) {
		outChildBlocked(child);
	}
}
