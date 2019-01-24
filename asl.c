#include "asl.h"

semd_t* getSemd(int *key){
	semd_t *iter;

	semd_t *s;
	s = NULL;
	list_for_each_entry(iter, &semd_h, s_next){

		if(iter->s_key == key){
			s = iter;
		}

	}

	return s;
}

void initASL(){

	INIT_LIST_HEAD(&semdfree_h);
	INIT_LIST_HEAD(&semd_h);

	semd_t *s;
	int i;

	for(i = 0; i < MAXPROC; i++){
		s = &semd_table[i];

		list_add(&s->s_next, &semdfree_h);
	}
}

void initSem(semd_t *sem, int *key){

	INIT_LIST_HEAD(&(sem->s_next));
	INIT_LIST_HEAD(&(sem->s_procQ));
	sem->s_key = key;
}

int insertBlocked(int *key,pcb_t* p){
	semd_t *s;
	s = getSemd(key);
	if(s != NULL){
		insertProcQ(&s->s_procQ, p);
		p->p_semkey = key;
		return FALSE;
	}

	if(list_empty(&semdfree_h)){
		return TRUE;
	} else {
		s = container_of(semdfree_h.next, semd_t, s_next);
		list_del(semdfree_h.next);

		initSem(s,key);

		insertProcQ(&s->s_procQ, p);

		semd_t *list_s;
		list_for_each_entry(list_s, &semd_h, s_next){
			if(s->s_key > list_s->s_key){
				list_add(&s->s_next, &list_s->s_next);
				p->p_semkey = key;
				return FALSE;
			}
		}

		list_add(&s->s_next, &semd_h);
		p->p_semkey = key;
		return FALSE;
	}

}

pcb_t* removeBlocked(int *key){
	semd_t *s;
	s = getSemd(key);
	if(s != NULL){
		pcb_t *p;
		p = container_of(s->s_procQ.next, pcb_t, p_next);
		list_del(s->s_procQ.next);
		if(emptyProcQ(&s->s_procQ)){
			list_del(&s->s_next);
			list_add(&s->s_next, &semdfree_h);
		}
		p->p_semkey = NULL;
		return p;
	}

	return NULL;

}

pcb_t* outBlocked(pcb_t *p) {

	int *semkey = p->p_semkey;
	pcb_t *tmp;
	semd_t *s = getSemd(semkey);

	if(s != NULL){
		tmp = outProcQ(&s->s_procQ, p);
		tmp->p_semkey = NULL;
		if(emptyProcQ(&s->s_procQ)){
			list_del(&s->s_next);
			list_add(&s->s_next, &semdfree_h);
		}
		return tmp;
	}

	return NULL;
}

pcb_t* headBlocked(int *key) {
	semd_t *s;
	s = getSemd(key);

	if(s != NULL && !emptyProcQ(&s->s_next)){
		pcb_t *tmp;
		tmp = container_of(s->s_procQ.next, pcb_t, p_next);
		return tmp;
	}

	return NULL;

}

void outChildBlocked(pcb_t *p) {
	pcb_t *child;
	outBlocked(p);
	list_for_each_entry(child, &p->p_child, p_sib){
		outChildBlocked(child);
	}
}