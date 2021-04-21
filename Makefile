# GRR20190367 Vinicius Tikara Venturi Date
CC = gcc
CFLAGS = -Wall -g

all: main

main: read.o

clean:
	rm -f *.o

purge: clean
	rm -f main