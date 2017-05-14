#!/bin/bash
#Makefile for generator and sauna binaries
all: gerador sauna

gerador: gerador.c utils.h
	gcc -Wall gerador.c -o gerador -lpthread
	
sauna: sauna.c utils.h
	gcc -Wall sauna.c -o sauna -lpthread

clean:
	rm -f /tmp/entrada /tmp/rejeitados
	rm -f gerador sauna
