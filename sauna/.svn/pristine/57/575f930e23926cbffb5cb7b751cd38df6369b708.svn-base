#!/bin/bash
#Makefile for Sauna
sauna: sauna.o vector_tid.o utils.o
	gcc -Wall -pthread sauna.o vector_tid.o utils.o -o sauna -lm
vector_tid.o: vector_tid.c vector_tid.h
	gcc -Wall -pthread -c vector_tid.c
utils.o: utils.c utils.h
	gcc -Wall -c utils.c
sauna.o: sauna.c vector_tid.h utils.h
	gcc -Wall -pthread -c sauna.c
clean:
	rm -f sauna
	rm -f vector_tid.o
	rm -f sauna.o
