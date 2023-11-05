CC = gcc
CFLAGS = -pedantic -Wall -Wextra
all: utility/utility.o navi porti master
navi: navi/navi.o 
	$(CC) $(CFLAGS) utility/utility.o navi/navi.o -o navi/navi -lm
navi/navi.o: utility/utility.o navi/navi.c 
	$(CC) $(CFLAGS) -c  navi/navi.c -o navi/navi.o -lm
porti: porti/porti.o 
	$(CC) $(CFLAGS) utility/utility.o porti/porti.o -o porti/porti
porti/porti.o: utility/utility.o porti/porti.c 
	$(CC) $(CFLAGS) -c  porti/porti.c -o porti/porti.o -lm
master: master/master.o 
	$(CC) $(CFLAGS) utility/utility.o master/master.o -o master/master
master/master.o: utility/utility.o master/master.c 
	$(CC) $(CFLAGS) -c  master/master.c -o master/master.o -lm
utility/utility.o: 
	$(CC) $(CFLAGS) -c utility/utility.c -o utility/utility.o -lm
clean: 
	rm -f utility/utility navi/navi porti/porti master/master utility/*.o navi/*.o porti/*.o master/*.o
run: all 
	./master/master
log: all 
	./master/master > log.txt