#ifndef __GAOS__TYPES
#define __GAOS__TYPES

#ifndef UARMTYPES_H 			//gestione per conflitti di definizione delle variabili che seguono
#define NULL ( (void *) 0)
typedef unsigned int state_t;
#endif


/* struct clist definition. It is at the same time the type of the tail
 pointer of the circular list and the type of field used to link the elements */

typedef struct clist {
    struct clist *next;
}clist;

/* process control block type */

typedef struct pcb_t {
    struct pcb_t *p_parent; 			/* pointer to parent */
    struct semd_t *p_cursem; 			/* pointer to the semd_t on
                              				which process blocked */
    state_t p_s; 						/* processor state */
    struct clist p_list;			 	/* process list */
    struct clist p_children; 			/* children list entry point*/
    struct clist p_siblings; 			/* children list: links to the siblings */
}pcb_t; 

#endif //__GAOS__TYPES
