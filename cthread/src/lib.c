
#include <stdio.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <ucontext.h>

int next_tid = 1;
int initialize = 0;
int debug = 0;

PFILA2 apto, bloqueado, termino;

TCB_t *current_thread; //thread atual


ucontext_t terminate_context;
ucontext_t escalonador_context;


TCB_t* getThreadTarget(int tid);
int ThreadExists(int tid);
TCB_t* getThreadFila(int tid, PFILA2 fila);
int dispatcher(int tid);
int escalonador();
TCB_t* escalonador_semaforo(PFILA2 fila_sem);
TCB_t * getRemoveFila(int tid, PFILA2 fila);
void terminateCallBack();

//INIT ALL QUEUES AND STRUCTURES FOR QUEUES
void initFilas(){
    apto = malloc(sizeof(FILA2));
    bloqueado = malloc(sizeof(FILA2));
    termino = malloc(sizeof(FILA2));

    CreateFila2(apto);
    CreateFila2(bloqueado);
    CreateFila2(termino);
    if(debug ==1){
      printf("Filas inicializadas com sucesso\n" );
    }
}

TCB_t* createThread(){

    TCB_t *new_thread = malloc(sizeof(TCB_t));
    char _stack[SIGSTKSZ];
    if(new_thread != NULL){
        new_thread->tid = next_tid;
        new_thread->state = PROCST_CRIACAO;
        new_thread->prio = 0;
        new_thread->esperada_pela = -1;
        new_thread->esperando_por = -1;
        next_tid++;
        if(getcontext(&new_thread->context) == RETURN_SUCCESS){
        }
        new_thread->context.uc_stack.ss_sp = malloc(SIGSTKSZ);
        new_thread->context.uc_stack.ss_size = sizeof(_stack);
        new_thread->context.uc_link = &terminate_context;
        if(debug ==1){
          printf("Tread %d criada com sucesso\n", new_thread->tid);
        }
        return new_thread;
    }
    return NULL;
}

//INITIALIZE MAIN THREAD
int initMainThread(){

    //main context stuff
    TCB_t *main_thread = malloc(sizeof(TCB_t));

    if(main_thread != NULL){
        //init main TCB
        main_thread->tid = 0;
        main_thread->state = PROCST_EXEC;
        main_thread->prio = 0;

        if (getcontext(&main_thread->context) == RETURN_SUCCESS){
            current_thread = main_thread;
            return RETURN_SUCCESS;
            if(debug ==1){
              printf("Tread main criada com sucesso\n");
            }
        }
    }
    return RETURN_ERROR;
}

//INITIALIZE ALL STRUCTURES
void init(){

    //initialize all queues and main thread
    initFilas();
    initMainThread();

    // create scheduler context
    if (getcontext(&escalonador_context) == RETURN_SUCCESS){
        char escalonador_stack[SIGSTKSZ];
        escalonador_context.uc_link = NULL;
        escalonador_context.uc_stack.ss_sp = malloc(SIGSTKSZ);
        escalonador_context.uc_stack.ss_size = sizeof(escalonador_stack);
        makecontext(&escalonador_context, (void (*)(void))escalonador, 0);
    }

    // create terminate callback
    if (getcontext(&terminate_context) == RETURN_SUCCESS){
	char terminate_stack[SIGSTKSZ];
        terminate_context.uc_link = NULL;
        terminate_context.uc_stack.ss_sp = malloc(SIGSTKSZ);
        terminate_context.uc_stack.ss_size = sizeof(terminate_stack);
        makecontext(&terminate_context, (void (*)(void))terminateCallBack, 0);
    }

    if(debug ==1){
      printf("Inicializaçao das estruturas feita com sucesso\n");
    }

    initialize = 1;
}


int ccreate (void* (*start)(void*), void *arg, int prio) {

	TCB_t *new_thread;
	// Testa se as estruturas foram inicializadas
	if(initialize == 0){
		init();
	}
	//Cria um nova thread
	new_thread = createThread();
	//se a nova thread nao for null, muda o estado para apto e move a thread para a fila de apto
	if(new_thread != NULL){
		makecontext(&new_thread->context, (void (*)(void))start, 1, arg);
		new_thread->state = PROCST_APTO;

		if(AppendFila2(apto, new_thread) == RETURN_SUCCESS){
      if(debug ==1){
	printf("CCREATE rodou e inseriu a thread %d na fila de apto\n", new_thread->tid);
      }
		return new_thread->tid;
		}
		else // Nao fez o append
		return RETURN_ERROR;
	}
	else //thread == null
		return RETURN_ERROR;
}

void terminateCallBack() {
	TCB_t *end_thread = current_thread;
	TCB_t *blocked_thread;
	end_thread->state = PROCST_TERMINO;
	end_thread->prio = stopTimer()/10000;
	if(debug ==1){
		printf("Terminate rodou e inseriu a thread %d na fila de termino. e sua prioridade eh %d \n", end_thread->tid, end_thread->prio);
	}


	////printf("Vai encerrar a thread de tid = %d\n", end_thread->tid);
	// restore a thread that may be waiting for this tid
	if (end_thread->esperada_pela != -1) {
		//CHECKS IF BLOCKED THREAD IS IN BLOCKED QUEUES
		blocked_thread = getRemoveFila(end_thread->esperada_pela, bloqueado);
		if (blocked_thread != NULL){
			blocked_thread->state = PROCST_APTO;
			blocked_thread->esperando_por = -1;
			AppendFila2(apto, blocked_thread);
		}
	}
    AppendFila2(termino, end_thread);
    //escalonador();
	if(debug ==1){
	      printf("Fim do Terminate, agora vai chamar o escalonador\n");
	}

	//swapcontext(&current_thread->context, &escalonador_context);
	escalonador();
	if(debug ==1){
		printf("Fim do Terminate, fez chamada pro escalonador\n");
	}
}

TCB_t * getRemoveFila(int tid, PFILA2 fila){
    //procurar na queue a thread com identificador = tid
    //PNODE2 first_fila;
    TCB_t *first_node;
    int first_node_tid;

    // pega o tid do primeiro elemento da queue
    FirstFila2(fila);
    first_node = (TCB_t *)GetAtIteratorFila2(fila);
    first_node_tid = first_node->tid;

    //se é o tid que procuramos, retorna a thread
    if(first_node_tid == tid){
        DeleteAtIteratorFila2(fila);
        return first_node;
    }else{ //se não, repete o processo para os próximos elementos
        // até encontrar a thread com o tid
        while(NextFila2(fila) == RETURN_SUCCESS){
            first_node = (TCB_t *)GetAtIteratorFila2(fila);
            first_node_tid = first_node->tid;
            if(first_node_tid == tid){
                DeleteAtIteratorFila2(fila);
                return first_node;
            }
        }
    return NULL;
    }
}

int escalonador(){

	// Vai para o primeiro da fila		1
	//(*) pega o tid do processo e a prioridade dele		2
	// vai para o próximo elemento da fila e compara as prioridades		3
		// se a prioridade for maior: (valor menor)		4
		// repete (*)
	// retorna o tid do processo na memória
getcontext(&escalonador_context);

	TCB_t	*temp, *maior_prio;

	if (FirstFila2(apto) == RETURN_SUCCESS)
        	maior_prio = (TCB_t *)GetAtIteratorFila2(apto);
	else
		return RETURN_ERROR;
	while (NextFila2(apto) == RETURN_SUCCESS)
	{
		temp = (TCB_t *)GetAtIteratorFila2(apto);
		if(temp->prio < maior_prio->prio) // se a prioridade for maior
			maior_prio = temp;
	}
	// Aqui, maior_prio é o processo que deve executar agora.
  if(debug ==1){
    printf("Rodou o escalonador, thread escolhida foi a %d, sua prioridade eh %d \n", maior_prio->tid, maior_prio->prio);
  }
	dispatcher(maior_prio->tid);
  return 0;

}


int dispatcher(int tid){

	if (initialize == 0){
		init();
	}

  	// getcontext(&dispatcher_context); //???????

	TCB_t* next_thread;
	/// \acha a thread na fila de aptos:
	FirstFila2(apto);
        next_thread = (TCB_t *) GetAtIteratorFila2(apto);
        if(next_thread->tid != tid)
	{
      		while (NextFila2(apto) == RETURN_SUCCESS)
      		{
      		    	next_thread = (TCB_t *)GetAtIteratorFila2(apto);
      		    	if (next_thread->tid == tid)
			{
				if(debug ==1){
				  printf("Dispatcher encontrou a thread escalonada %d", next_thread->tid);
				}
		      		break;
		        }
      		}
      	}

 	if(DeleteAtIteratorFila2(apto) == RETURN_SUCCESS)
	{
                //outro: First time, it saves the context where the next thread has not ended yet
                //outro: when it returns by next link (which means that next has ended), the callback function will end this thread
                //getcontext(&terminate_context);  //????????

                if (next_thread->state != PROCST_EXEC)
		{
                  	next_thread->state = PROCST_EXEC;

                  	current_thread = next_thread;
                  	setcontext(&next_thread->context);
                  	///////////// time init
                  	startTimer();

                    	if(debug ==1){
                      		printf("startou o timer a partir do dispatcher");
                    	}
                  ///////////// time init
                }else
		{
		        if(debug ==1){
		            	printf("Entrou no terminate a partir do dispatcher -- não deveria acontecer");
                  	}

                	//terminateCallBack();
                }
	}

    return RETURN_SUCCESS;
}

int cyield() {

	if(initialize == 0)
	{
      		init();
  	}

  	current_thread->state = PROCST_APTO;
	current_thread->prio = stopTimer()/10000;
 	if(AppendFila2(apto, current_thread) == RETURN_SUCCESS)
	{ // retorna 0 caso tenha obtido sucesso, igual ao AppendFila2
    		if (swapcontext(&current_thread->context, &escalonador_context) == RETURN_ERROR)
    		//if (escalonador() == RETURN_ERROR)
		{
	        	return RETURN_ERROR;
    		}
		else
		{
			if(debug ==1){
				printf("fim da yield. tid da current = %d ", current_thread->prio);
			}
        		return RETURN_SUCCESS;
   		}
  	}
return RETURN_ERROR;
}

int cjoin(int tid) {
	if(debug ==1){
		printf("iniciando cjoin com parametro tid = %d\n", tid);
	}
	TCB_t *thread_target , *temp; //thread that will be waited

	if (!initialize){
		init();
  	}

    	if (FirstFila2(termino) == RETURN_SUCCESS){
            temp = (TCB_t *)GetAtIteratorFila2(termino);
            if(temp->tid == tid){
	      if(debug ==1){
		printf("encontrou o thread esperada na fila de termino = %d\n", tid);
	      }
              return RETURN_SUCCESS;
            }

    }
    while (NextFila2(termino) == RETURN_SUCCESS)
    {
      temp = (TCB_t *)GetAtIteratorFila2(termino);
      if(temp->tid == tid){
	if(debug ==1){
	   printf("encontrou o thread esperada na fila de termino = %d\n", tid);
	}
        return RETURN_SUCCESS;
      }
    }

  //verify if the given thread exists
	if (ThreadExists(tid))
	{
	      thread_target = getThreadTarget(tid); //gets the thread target

	      //if the thread_target already is waited by some tcb, return error,
	      //else, sets the wait_by atribute to the current tcb tid
		if (thread_target->esperada_pela == -1)
		{
			thread_target->esperada_pela = current_thread->tid;
			current_thread->esperando_por = thread_target->tid;
			//sets the current_thread to blocked state
			current_thread->state = PROCST_BLOQ;
			//put the current_thread in the blocked queue
			AppendFila2(bloqueado,current_thread);
			current_thread->prio = stopTimer()/10000;
			if(debug ==1)
			{
				printf("Thread de tid = %d inserida na fila de Bloqueados com prio %d \n",current_thread->tid, current_thread->prio);
			}
			if(swapcontext(&current_thread->context, &escalonador_context) == -1)
			{
			//if(escalonador == RETURN_ERROR){  //????????
				return RETURN_ERROR;
			}
			//escalonador();
			  return RETURN_SUCCESS;
			if(debug ==1)
			{
				printf("Fim da cjoin. \n");
			}
		}
		else
		{
			if(debug ==1)
			{
				printf("Essa thread ja esta sendo esperada por outra thread...\n");
			}
	      		return RETURN_ERROR;
	  	}
	}
	  //send error message warning that the tid doesn't exists
	else
	{
		if(debug ==1){
			printf("tid invalido ou inexistente no parametro da cjoin...\n\n");
		}
		return RETURN_ERROR;
	}
}

int ThreadExists(int tid){
    if (getThreadTarget(tid) != NULL)
        return 1;
    return 0;
}

TCB_t* getThreadTarget(int tid){
    TCB_t *thread;

    //cheks for the tid in all queues
    thread = getThreadFila(tid, apto);
    if (thread != NULL){
        //printf("ENCONTROU NA FILA DE APTOS\n");
        return thread;
    }
    thread = getThreadFila(tid, bloqueado);
    if (thread != NULL){
        //printf("ENCONTROU NA FILA DE BLOQUEADOS\n");
        return thread;
    }

    // thread = getThreadFila(tid, termino);
    // if (thread != NULL){
    //     //printf("ENCONTROU NA FILA DE BLOQUEADOS\n");
    //     return thread;
    // }

    // thread = getThreadFromSemaphoreQueue(tid);
    // if (thread != NULL){
    //     //printf("ENCONTROU NA FILA DO SEMAFORO\n");
    //     return thread;
    // }
    return NULL;
}

TCB_t* getThreadFila(int tid, PFILA2 fila){
    TCB_t *thread;
    //sets the iterator in the start of the queue
    if (FirstFila2(fila) == RETURN_SUCCESS){
        thread = (TCB_t *) GetAtIteratorFila2(fila);
        if(thread->tid == tid){
            return thread;
        }
        //search for the given tid in the queue
        while (NextFila2(fila) == RETURN_SUCCESS){
            thread = (TCB_t *)GetAtIteratorFila2(fila);
            //if the thread's tid is equal to the given tid, returns the thread
            if (thread->tid == tid){
                return thread;
            }
        }
        //if doesn't foud, return NULL
        return NULL;
    }
    //if the first element doesn't exists, return NULL
    return NULL;
}

int csem_init(csem_t *sem, int count) {
	if(!initialize){
		init();
	}

	if (sem == NULL)
		return RETURN_ERROR;

	sem->count = count; // (conforme definicao)
	sem->fila = malloc(sizeof(FILA2));

	CreateFila2(sem->fila);

	return RETURN_SUCCESS;
}

int cwait(csem_t *sem) {

	if (sem == NULL) {
		return RETURN_ERROR;
	}

	if (sem->count > 0) {
		(sem->count)--;
		return RETURN_SUCCESS;
	} else { //count <= 0
		(sem->count)--;

		AppendFila2(bloqueado, (void *) current_thread);
		current_thread->state = PROCST_BLOQ;

		AppendFila2(sem->fila, (void *) current_thread);

	     	//escalonador();
		swapcontext(&current_thread->context, &escalonador_context);
		return RETURN_SUCCESS;
	}
}


TCB_t* escalonador_semaforo(PFILA2 fila_sem){

	TCB_t	*temp, *maior_prio;

	if (FirstFila2(fila_sem) == RETURN_SUCCESS)
        	maior_prio = (TCB_t *)GetAtIteratorFila2(fila_sem);
	else
		return NULL;
	while (NextFila2(fila_sem) == RETURN_SUCCESS)
	{
		temp = (TCB_t *)GetAtIteratorFila2(fila_sem);
		if(temp->prio < maior_prio->prio) // se a prioridade for maior
			maior_prio = temp;
	}
	// Aqui, maior_prio é o processo que deve executar agora.
  return maior_prio;

}

int csignal(csem_t *sem) {


  if (sem == NULL) {
      return RETURN_ERROR;
  }

  sem->count = sem->count + 1;

  TCB_t *thread = escalonador_semaforo(sem->fila);

  if (thread != NULL) {
      DeleteAtIteratorFila2(sem->fila);

      FirstFila2(bloqueado);
      while ((((TCB_t *) GetAtIteratorFila2(bloqueado))->tid) != thread->tid) {
          	if(NextFila2(bloqueado) != RETURN_SUCCESS)  //aqui dá seg fault no teste3
			return RETURN_ERROR;
	}
      	DeleteAtIteratorFila2(bloqueado);

	if(AppendFila2(apto, thread) == RETURN_SUCCESS){
	thread->state = PROCST_APTO;
	}else{
		return RETURN_ERROR;
      }
  }

  return RETURN_SUCCESS;
}

int cidentify (char *name, int size) {
	strncpy (name, "Henrique Mendes de Moura 252859.\n Luiz Miguel Kruger 228271 \n Vitor dos Santos Cavalheiro 275540", size);
	return 0;
}
