#ifndef _CONST_H
#define _CONST_H

/****************************************************************************
 *
 * This header file contains the global constant & macro definitions.
 *
 ****************************************************************************/

/* Maxi number of overall (eg, system, daemons, user) concurrent processes */
#define MAXPROC 20

#define UPROCMAX 3  /* number of usermode processes (not including master proc
		       and system daemons */

#define	HIDDEN static
#define	TRUE 	1
#define	FALSE	0
#define ON 	1
#define OFF 	0
#define EOS '\0'


/* Addresses for new and old areas (where new and old processor states are
   stored on exceptions) */
#define INT_NEWAREA 0x2000008c
#define INT_OLDAREA 0x20000000
#define TLB_NEWAREA 0x200001a4
#define TLB_OLDAREA 0x20000118
#define PGMTRAP_NEWAREA 0x200002bc
#define PGMTRAP_OLDAREA 0x20000230
#define SYSBK_NEWAREA 0x200003d4
#define SYSBK_OLDAREA 0x20000348

#define DEV_PER_INT 8 /* Maximum number of devices per interrupt line */

#define NULL ((void *) 0)

#define CR 0x0a   /* carriage return as returned by the terminal */

#endif
