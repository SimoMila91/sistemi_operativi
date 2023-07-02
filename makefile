CC = gcc
CFLAGS = -Wall -Wextra

all: navi porti master

navi: navi.o
	$(CC) $(CFLAGS) -o navi navi.o

porti: porti.o
	$(CC) $(CFLAGS) -o porti porti.o

master: master.o
	$(CC) $(CFLAGS) -o master master.o

navi.o: navi/navi.c
	$(CC) $(CFLAGS) -c navi/navi.c

porti.o: porti/porti.c
	$(CC) $(CFLAGS) -c porti/porti.c

master.o: master/master.c
	$(CC) $(CFLAGS) -c master/master.c

clean:
	rm -f navi porti master *.o

run: all 
  ./master

log: all
	./master > log.txt