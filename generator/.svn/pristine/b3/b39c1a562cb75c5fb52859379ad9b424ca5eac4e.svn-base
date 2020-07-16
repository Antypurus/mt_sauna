#!/bin/bash
#Makefile for Gerador
gerador: gerador.o utils.o
	gcc -Wall -pthread gerador.o utils.o -o gerador -lm
gerador.o: gerador.c
	gcc -Wall -pthread -c gerador.c
utils.o: utils.c utils.h
	gcc -Wall -c utils.c
clean:
	rm -f gerador
	rm -f gerador.o
	rm -f utils.o
