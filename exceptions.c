#include "asl.e"
#include "pcb.h"

#include <libuarm.h>

// ---- implementa qui le syscall


void sysBpHandler(){
	
	//fa tanti controlli
	
	//in base al registro chiama le syscall
	switch(){
		case CREATEPROCESS:
		case...
		case....
		case.....
		...
	}
}



//Che palle... questi due fanno più o meno le stesse cose, cambia quale system call ha chiamato l'errore, per il primo SYS5 e il secondo SYS6

void pgmTrapHandler(){
	
}

void tlbHandler(){
	
}

