CC = gcc
CFLAGS = -pedantic -Wall -Wextra
all: navi porti master
navi: navi/navi.o 
	$(CC) $(CFLAGS) utility/utility.o navi/navi.o -o navi/navi -lm
navi/navi.o: utility/utility.o navi/navi.c 
	$(CC) $(CFLAGS) -c utility/utility.o navi/navi.c -o navi/navi.o -lm
porti: porti/porti.o 
	$(CC) $(CFLAGS) utility/utility.o porti/porti.o -o porti/porti
porti/porti.o: utility/utility.o porti/porti.c 
	$(CC) $(CFLAGS) -c utility/utility.o porti/porti.c -o porti/porti.o -lm
master: master/master.o 
	$(CC) $(CFLAGS) utility/utility.o master/master.o -o master/master
master/master.o: utility/utility.o master/master.c 
	$(CC) $(CFLAGS) -c utility/utility.o master/master.c -o master/master.o -lm
utility/utility.o: utility/utility.c
	$(CC) $(CFLAGS) -c utility/utility.c -o utility/utility.o -lm

clean: 
	rm -f navi/navi porti/porti master/master navi/.o porti/.o master/*.o
run: all 
	./master/master
log: all 
	./master/master > log.txt