CC = gcc
CFLAGS = -pedantic -Wall -Wextra 

all: navi porti master.o
	gcc  master/master.o -o run   navi/navi.o porti/porti.o -lm
navi: navi.o
	$(CC) $(CFLAGS)  navi/navi.o -o navi/navi -lm

porti: porti.o
	$(CC) $(CFLAGS)  porti/porti.o -o porti/porti

master: master.o
	$(CC) $(CFLAGS)  master/master.o -o master/master

navi.o: 
	$(CC) $(CFLAGS) -c navi/navi.c -o navi/navi.o -lm

porti.o: 
	$(CC) $(CFLAGS) -c porti/porti.c -o porti/porti.o -lm

master.o: 
	$(CC) $(CFLAGS) -c master/master.c -o master/master.o -lm

clean:
	rm -f navi porti master *.o

run: all 
	./master

log: all
	./master > log.txt