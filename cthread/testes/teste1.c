/*
**
** teste1.c
** Teste da Biblioteca cthreads
**
** Instituto de Informática - UFRGS
** Sistemas Operacionais I N 2018/1
** Prof. Sérgio Luis Cechin
**
** Caetano Jaeger
** Felipe Comerlato
** Leonardo Eich
*/

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

#include "../include/cdata.h"
#include "../include/cthread.h"
#include "../include/support.h"

void hello();
void space();
void world();
void excl();

int thello, tspace, tworld, texcl;

void hello(){
	cjoin(tworld);
	printf("hello\n");
	printf("sou a thread hello com tid: %d\n", thello);
}

void space(){
	cjoin(tworld);
	printf("_\n");
	printf("sou a thread space com tid: %d\n", tspace);
}

void world(){
	cjoin(thello);
	printf("world\n");
	cyield();
	printf("sou a thread world com tid: %d\n", tworld);
}

void excl(){
	cjoin(texcl);
	printf("!\n");
	printf("sou a thread excl com tid: %d\n", texcl);
}

int main()
{
	printf("criando uma thread para a funcao hello\n");
	thello = ccreate((void *)hello, NULL, 0);

	printf("criando uma thread para a funcao space\n");
	tspace = ccreate((void *)space, (void *)NULL, 0);

	printf("criando uma thread para a funcao world\n");
	tworld = ccreate((void *)world, (void *)NULL, 0);

	printf("criando uma thread para a funcao excl\n");
	texcl = ccreate((void *)excl, (void *)NULL, 0);

	printf("criando 20 threads para a funcao hello\n");
	int i = 0;
	for (i = 0; i < 20; ++i)
	{
		ccreate((void *)hello, NULL, 0);
	}

	printf("dando 10 yields na main\n");

	for (i = 0; i < 10; ++i)
	{
		cyield();
	}

	char * name = (char*) malloc(sizeof(char)*200);
	cidentify(name, 200);
	printf("%s\n", name);

	printf("main terminando\n");

	return 0;
}
