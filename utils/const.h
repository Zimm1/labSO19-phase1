#ifndef _CONST_H
#define _CONST_H

/**************************************************************************** 
 *
 * This header file contains the global constant & macro definitions.
 * 
 ****************************************************************************/

/* New processor state locations */
#define SYSBP_NEW_AREA 	0x200003D4
#define TRAP_NEW_AREA 	0x200002BC
#define TLB_NEW_AREA 		0x200001A4
#define INT_NEW_AREA    0x2000008C

/* Old processor state locations */
#define SYSBP_OLD_AREA 	0x20000348
#define TRAP_OLD_AREA 	0x20000230
#define TLB_OLD_AREA 		0x20000118
#define INT_OLD_AREA    0x20000000

/* CP0 status register */
#define INT_MASK_ON_OR				0x0000FF15
#define INT_TIMER_MASK_ON_OR	0x00000215
#define VM_OFF_AND						0xF8FFFFFF
#define KERNEL_ON_AND					0xFFFFFFD5
#define TIMER_ON_OR						0x08000000

/* Devices */
#define COMMAND_REG_OFFSET 	0x00000004
#define TERM_STATUS_READ 		0x00000000
#define TERM_COMMAND_READ 	0x00000004
#define TERM_STATUS_WRITE 	0x00000008
#define TERM_COMMAND_WRITE 	0x0000000C

/* Exc Cause */
#define EXC_CODE_MASK		0x00003C
#define EXC_CODE_SHIFT	2
#define EXC_SYS					8
#define EXC_BP					9

/* Syscalls */
#define SYS_TERMINATE_PROCESS	3

#define FRAMESIZE 					4096
#define TIME_SLICE 					3000 /* 3 milliseconds */
#define SCHED_PSEUDO_CLOCK 	10

#define	HIDDEN 	static
#define NULL 		((void *) 0)
#define	TRUE 		1
#define	FALSE		0
#define ON 			1
#define OFF 		0
#define EOS 		'\0'
#define CR 			0x0a   /* carriage return as returned by the terminal */

/* Maximum number of devices per interrupt line */
#define DEV_PER_INT 8
/* Max number of overall (eg, system, daemons, user) concurrent processes */
#define MAXPROC 	20
/* number of usermode processes (not including master proc and system daemons */
#define UPROCMAX 	3  

#endif
