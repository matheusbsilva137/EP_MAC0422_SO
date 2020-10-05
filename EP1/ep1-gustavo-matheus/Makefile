all: ep1 bccsh

ep1: ep1.c escalonador.h
	gcc ep1.c -o ep1 -lpthread

bccsh: bccsh.c -lreadline
	gcc bccsh.c -o bccsh -lreadline
