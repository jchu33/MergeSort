CC = gcc -g
CFLAGS = -g3 -std=c99 -pedantic -Wall
HWK3 = /c/cs223/Hwk3
HWK4 = /c/cs223/Hwk4



Merge16: Merge16.o Queue.o getLine.o
	${CC} ${CFLAGS} -o Merge16 Merge16.o Queue.o getLine.o

Merge16.o: Merge16.c Queue.h getLine.h
	${CC} ${CFLAGS} -c Merge16.c

Queue.o: Queue.c Queue.h
	${CC} ${CFLAGS} -c Queue.c
