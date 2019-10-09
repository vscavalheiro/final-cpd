#
# Makefile para programas de teste
#

CC=gcc
LIB_DIR=../lib

all: teste1 teste2 teste3

teste1: teste1.c $(LIB_DIR)/libcthread.a
	$(CC) -o teste1 teste1.c -L$(LIB_DIR) -lcthread -Wall

teste2: teste2.c $(LIB_DIR)/libcthread.a
	$(CC) -o teste2 teste2.c -L$(LIB_DIR) -lcthread -Wall

teste3: teste3.c $(LIB_DIR)/libcthread.a
	$(CC) -o teste3 teste3.c -L$(LIB_DIR) -lcthread -Wall

clean:
	rm -rf teste1 teste2 teste3 *.o *~
