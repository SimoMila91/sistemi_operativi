CC = gcc
CFLAGS = -pedantic -Wall -Wextra 

all: navi porti master
	$(CC) $(CFLAGS) -o master navi/navi.o porti/porti.o master/master.o
navi: 
	$(CC) $(CFLAGS) -c navi/navi.c -o navi/navi.o

porti: 
	$(CC) $(CFLAGS) -c porti/porti.c -o porti/porti.o

master: 
	$(CC) $(CFLAGS) -c master/master.c -o master/master.o

clean:
	rm -f navi porti master *.o

run: all 
	./master

log: all
	./master > log.txt