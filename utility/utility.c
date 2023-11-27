#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/sem.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "../macro/macro.h"
#include "utility.h"


void decreaseSem (struct sembuf sops, int sem_id, int sem_num){
    sops.sem_num = sem_num;
    sops.sem_op = -1; 
    sops.sem_flg = 0; 

    semop(sem_id, &sops, 1); 

}

void increaseSem (struct sembuf sops, int sem_id, int sem_num){
    sops.sem_num = sem_num;
    sops.sem_op = 1; 
    sops.sem_flg = 0; 

    semop(sem_id, &sops, 1); 

}


void waitForZero (struct sembuf sops, int sem_id, int sem_num){
    sops.sem_num = sem_num;
    sops.sem_op = 0; 
    sops.sem_flg = 0; 

    semop(sem_id, &sops, 1); 

}


void printTest(int riga){
    char *string;
    int numBytes;
    string = malloc(50);
    numBytes = sprintf(string, "===========> %d\n", riga);
    fflush(stdout);
    write(1, string, numBytes); 
    free(string);
}

void printTestDue(int numBytes, char *string){
    
    fflush(stdout);
    write(1, string, numBytes); 

}
