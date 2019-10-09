/*
 * cdata.h: arquivo de inclus�o de uso apenas na gera��o da libpithread
 *
 * VERS�O: 29/03/2019
 *
 */
#ifndef __cdata__
#define __cdata__
#include "ucontext.h"


#define RETURN_ERROR -1
#define RETURN_SUCCESS 0

/* Estados das threads
*/
#define	PROCST_CRIACAO	4 /* Processo em estado sendo criada */
#define	PROCST_APTO	0		/* Processo em estado apto a ser escalonado */
#define	PROCST_EXEC	1		/* Processo em estado de execu��o */
#define	PROCST_BLOQ	2		/* Processo em estado bloqueado */
#define	PROCST_TERMINO	3	/* Processo em estado de terminado */


typedef struct s_TCB {
	/* OS CAMPOS ABAIXO N�O PODEM SER ALTERADOS OU REMOVIDOS
	*/
	int		tid; 		// identificador da thread
	int		state;		// estado em que a thread se encontra (CONFORME DEFINES ANTERIORES)
	int 		prio;		// Prioridade associada a thread
	ucontext_t 	context;	// contexto de execu��o da thread (SP, PC, GPRs e recursos)

	/* A PARTIR DESSE PONTO PODEM SER ACRESCENTADOS CAMPOS NECESS�RIOS AO DESENVOLVIMENTO DA BIBLIOTECA
	*/
	int esperando_por;
	int esperada_pela;

} TCB_t;


#endif
