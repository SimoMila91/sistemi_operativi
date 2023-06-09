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
    TEST_ERROR;
}

void increaseSem (struct sembuf sops, int sem_id, int sem_num){
    sops.sem_num = sem_num;
    sops.sem_op = 1; 
    sops.sem_flg = 0; 

    semop(sem_id, &sops, 1); 
    TEST_ERROR;
}


void waitForZero (struct sembuf sops, int sem_id, int sem_num){
    sops.sem_num = sem_num;
    sops.sem_op = 0; 
    sops.sem_flg = 0; 

    semop(sem_id, &sops, 1); 
    TEST_ERROR;
}