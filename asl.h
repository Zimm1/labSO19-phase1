#ifndef ASL_H
#define ASL_H

#include <types_rikaya.h>
#include <const.h>
#include <pcb.h>

semd_t semd_table[MAXPROC];

struct list_head semdfree_h;

struct list_head semd_h;


/* ASL handling functions */
semd_t* getSemd(int *key);
void initASL();

int insertBlocked(int *key,pcb_t* p);
pcb_t* removeBlocked(int *key);
pcb_t* outBlocked(pcb_t *p);
pcb_t* headBlocked(int *key);
void outChildBlocked(pcb_t *p);

void initSem(semd_t *sem, int *key);

#endif
