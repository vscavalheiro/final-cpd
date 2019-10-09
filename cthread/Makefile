#
# Makefile de EXEMPLO
#
# OBRIGATÓRIO ter uma regra "all" para geração da biblioteca e de uma
# regra "clean" para remover todos os objetos gerados.
#
# É NECESSARIO ADAPTAR ESSE ARQUIVO de makefile para suas necessidades.
#  1. Cuidado com a regra "clean" para não apagar o "support.o"
#
# OBSERVAR que as variáveis de ambiente consideram que o Makefile está no diretótio "cthread"
#

CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src

all: octhread alib

#cria objeto cthread.o e move para pasta bin
octhread: $(SRC_DIR)/lib.c
	$(CC) -c $^ -Wall
	mv lib.o $(BIN_DIR)

#gera a biblioteca libcthread.a e move para pasta lib
alib: $(BIN_DIR)/lib.o $(BIN_DIR)/support.o
	ar crs libcthread.a $^
	mv libcthread.a $(LIB_DIR)

#apaga os arquivos .o e a biblioteca libcthread.a de seus diretorios
clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/lib.o
