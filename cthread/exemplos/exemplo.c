
/*
 *	Programa de exemplo de uso da biblioteca cthread
 *
 *	Vers�o 1.0 - 14/04/2016
 *
 *	Sistemas Operacionais I - www.inf.ufrgs.br
 *
 */

#include "../include/support.h"
#include "../include/cthread.h"
#include <stdio.h>

void* func0(void *arg) {
	printf("\tEu sou a thread ID0 imprimindo %d\n", *((int *)arg));
	return;
}

void* func1(void *arg) {
	printf("\tEu sou a thread ID1 imprimindo %d\n", *((int *)arg));
	return;
}

int main(int argc, char *argv[]) {
	printf("\tEntrei na main");
	int	id0, id1;
	int i;

	id0 = ccreate(func0, (void *)&i, 0);
	id1 = ccreate(func1, (void *)&i, 1);

	printf("\tEu sou a main ap�s a cria��o de ID0 e ID1\n");
	
	fprintf(stderr, "\t cjoin de id0 = %d\n", cjoin(id0));
	printf("\tin between moves\n");
	fprintf(stderr, "\t cjoin de id1 = %d\n", cjoin(id1));

	fprintf(stderr, "\tEu sou a main voltando para terminar o programa\n");
return i;
}
