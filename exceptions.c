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



/* SYS1 : Crea un nuovo processo.
  * @param statep : stato del processore da cui creare il nuovo processo.
  * @return Restituisce -1 in caso di fallimento, mentre il PID (valore maggiore o uguale a 0) in caso di avvenuta creazione.
 */
int createProcess(state_t *statep){
	int i;
	pcb_t *p;

	/* In caso non ci fossero pcb liberi, restituisce -1 */
	if((p = allocPcb()) == NULL)
		return -1;
	else {
		/* Carica lo stato del processore in quello del processo */
		STST(&(p->p_state)); //Speriamo vada bene...

		/* Aggiorna il contatore dei processi e il pidCount (progressivo) */
		processCount++;
		pidCount++;
		p->p_pid = pidCount;

		/* Ricerca una cella vuota della tabella pcbused_table in cui inserire il processo appena creato */
		for(i=0; i<MAXPROC; i++)
			if(pcbused_table[i].pid == 0) break;

		/* Aggiorna la tabella dei pcb utilizzati, assegnando il giusto pid e il puntatore al pcb del processo creato */
		pcbused_table[i].pid = p->p_pid;
		pcbused_table[i].pcb = p;

		/* p diventa un nuovo figlio del processo chiamante */
		insertChild(currentProcess, p);

		insertProcQ(&readyQueue, p);

		return pidCount;
	}
}

//Semaphore Operation SYS3
void semaphoreOperation (int *semaddr, int weight){

	if (weight==0){
		SYSCALL(TERMINATEPROCESS, SYSCALL(GETPID)); //vediamo se possiamo farlo
	}
	else{

		(*semaddr) += weight;
		if(weight > 0){ //abbiamo liberato risorse
			if(*semaddr >= 0){

				// Se sem > risorse richieste dal primo bloccato --> sblocco processo
				pcb_t *p;
				p=headBlocked(semaddr);
				if(p!=NULL){
					if(p->p_resource<=weight){
						p = removeBlocked(semaddr);
						if (p != NULL){
							p->p_resource=0;
							insertProcQ(&readyQueue, p);
						}
					}
				}
			}
		}
		else{ // weight <0, abbiamo allocato risorse
			//When  resources  are  allocated,  if  the  value  of  the  semaphore  was  or
			//becomes negative, the requesting process should be blocked in the semaphore's queue.

			//se il semaforo era o diventa negativo ci blocchiamo, altrimenti modifichiamo il valore del semaforo

			if  (*semaddr <= 0)  {
				currentProcess->p_resource=weight;
				if(insertBlocked(semaddr, currentProcess))
					PANIC();	//currentProcess è dell'initial???
				currentProcess = NULL;
			}

		}
	}
}


//Che palle... questi due fanno più o meno le stesse cose, cambia quale system call ha chiamato l'errore, per il primo SYS5 e il secondo SYS6

void pgmTrapHandler(){
	
}

void tlbHandler(){
	
}

