#include "asl.e"
#include "pcb.h"

#include <libuarm.h>

HIDDEN state_t *sysBp_old = (state_t *) SYSBK_OLDAREA; //Cos'è???


// ---- implementa qui le syscall


void sysBpHandler(){
	
	int cause;
	int mode;
	
	//1-dovremmo salvare lo stato del registro
	
	//2-boh -> il tipo dice per evitare loop syscall :/
	currentProcess->p_s.cpsr += //qualcosa;
	
	//3-prendiamo il mode
	mode= ((sysBp_old->cpsr & STATUS_SYS_MODE) >> 0x3); //forse funziona -> STATUS_SYS_MODE in uarmConst.h
	
	//4-cause interrupt
	cause=getCAUSE();
	
	if(cause == EXC_SYSCALL){ //caso system call
		
		//controlla se è in user mode
		if(mode==TRUE){ //è definito da qualche parte il true?
			
			//controllo se è una delle 11 syscall
			if((sysBp_old->reg_a0 >= 1) && (sysBp_old->reg_a0 <= SYSCALL_MAX)){ //SYSCALL_MAX sta in const.h
			
				sysBp_old->CP15_Cause = setCAUSE(); //siamo sicuri non ci vadano parametri?
				
				//salva il sysbp old in pgmtrap old
				
				pgmTrapHandler();
				
			}
			else{
				//ERRORE!!! FACCIAMO UN ALTRA VOLTA!!!
			}
		}
		else{//caso kernel mode
			
			int ret;
			
			/* Salva i parametri delle SYSCALL */
			U32 argv1 = sysBp_old->a2;
			U32 argv2 = sysBp_old->a3;
			U32 argv3 = sysBp_old->a4;
			
			
			
			/* Gestisce ogni singola SYSCALL */
			switch(sysBp_old->a1)
			{
				case CREATEPROCESS:
					//currentProcess->p_state.reg_v0 = createProcess((state_t *) arg1);
					break;
					
				case TERMINATEPROCESS:
					//ris = terminateProcess((int) arg1);
					//if(currentProcess != NULL) currentProcess->p_state.reg_v0 = ris;
					break;
					
				case SEMOP:
					semaphoreOperation((int *) argv1, (int) argv2);
					break;
				
				case SPECSYSHDL:
					
					break;
					
				case SPECTLBHDL:
					
					break;
					
				case SPECPGMTHDL:
					
					break;
					
					
				case EXITTRAP:
					
					break;
					
				case GETCPUTIME:
					//currentProcess->p_state.reg_v0 = getCPUTime();
					break;
					
				case WAITCLOCK:
					//waitClock();
					break;
					
				case IODEVOP:
					
					break;
					
				case GETPID:
					currentProcess->p_state.reg_v0 = getPid();
					break;
					
				
				/*
					
				case WAITIO:
					currentProcess->p_state.reg_v0 = waitIO((int) arg1, (int) arg2, (int) arg3);
					break;
					
				case GETPPID:
					currentProcess->p_state.reg_v0 = getPpid();
					break;
					
				case SPECTLBVECT:
					specTLBvect((state_t *) arg1, (state_t *)arg2);
					break;
					
				case SPECPGMVECT:
					specPGMvect((state_t *) arg1, (state_t *)arg2);
					break;
					
				case SPECSYSVECT:
					specSYSvect((state_t *) arg1, (state_t *)arg2);
					break;
					
				 */
					
				default:
					/* Se non è già stata eseguita la SYS12, viene terminato il processo corrente */
					if(currentProcess->ExStVec[ESV_SYSBP] == 0)
					{
						int ris;
						
						ris = terminateProcess(-1);
						if(currentProcess != NULL) currentProcess->p_state.reg_v0 = ris;
					}
					/* Altrimenti viene salvata la SysBP Old Area all'interno del processo corrente */
					else
					{
						saveCurrentState(sysBp_old, currentProcess->sysbpState_old);
						LDST(currentProcess->sysbpState_new);
					}
			}
			
			scheduler();
			
		}
	}
	else{ //caso breakpoint
	
 
	}
}


//Semaphore Operation SYS3
void semaphoreOperation (int *semaddr, int weight){
	
	if (weight==0){
		SYSCALL(TERMINATEPROCESS, SYSCALL(GETPID)); //vediamo se possiamo farlo
	}
	else{
		(*semaddr) += weight; //vediamo se funziona
		
		if(*semaddr <=0){ // se il semaforo è minore di zero, non dovrei bloccare il processo al semaforo invece di sloccarlo?
			
			pcb_t *p;
			
			p = removeBlocked((S32 *) semaddr); // percé c'è il cast?
			/* Se è stato sbloccato un processo da un semaforo esterno */
			if (p != NULL)
			{
				/* Viene inserito nella readyQueue e viene aggiornata la flag isOnDev a FALSE */
				insertProcQ(&readyQueue, p);
				// p->p_cursem = NULL; non serve perchè lo fa dentro l'insert
			}
			
		}
		else{
			
			/* Inserisce il processo corrente in coda al semaforo specificato */
			if(insertBlocked((S32 *) semaddr, currentProcess)) PANIC();	//currentProcess è dell'initial???
			currentProcess = NULL;
			
		}
	}
	
}




//Che palle... questi due fanno più o meno le stesse cose, cambia quale system call ha chiamato l'errore, per il primo SYS5 e il secondo SYS6

void pgmTrapHandler(){
	
}

void tlbHandler(){
	
}

