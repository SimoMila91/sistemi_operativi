#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include "master.h"; 

int SO_PORTI; 
int SO_NAVI;  
double SO_LATO;  
int SO_BANCHINE; 
int SO_MERCI; 
int SO_SIZE; 
int SO_MAX_VITA; 
int SO_MIN_VITA; 
int SO_FILL; 

// port's variables 
int shmid_port; 
port* portList;

// ship's variables 
int shmid_nave;
ship* shipList;



int main(int argc, char **argv) {
    int i;
    char* args[3];
    char* idx_port[3*sizeof(int)+1];
    char* idx_ship[3*sizeof(int)+1];
    char* shmid_port_str[3*sizeof(int)+1];
    init_var();  // initialize of variables 
    args[0] = "porti";
   
    shmid_port = createSharedMemory(sizeof(port) * SO_PORTI); 
    portList = (port*)shmat(shmid_port, NULL, 0); 
    sprintf(shmid_port_str, "%d", shmid_port);
    args[2] = shmid_port_str;

    for(i=0; i < SO_PORTI; i++){
        pid_t pid = fork(); 
        switch (pid)
        {
        case -1:
            printf("error in fork of port's process\n" );
            exit(EXIT_FAILURE);
            break;
        case 0: 
            sprintf(idx_port, "%d", i);
            args[1] = idx_port; 
            execv("../porti/porti.c", args);
            TEST ERROR;
            exit(EXIT_FAILURE);
        default:
            //padre 
            break;
        }
    }



    args[0] = "navi";

    for(i = 0; i < SO_NAVI; i++){
        pid_t pid = fork(); 
            switch (pid)
            {
            case -1:
                printf("error in fork of ship's process\n" );
                exit(EXIT_FAILURE);
                break;
            case 0: 
                sprintf(idx_ship, "%d", i);
                args[1] = idx_ship; 
                execv("../navi/navi.c", args);
                TEST ERROR;
                exit(EXIT_FAILURE);
            default:
                //padre 
                break;
            }
    }
}

int createSharedMemory(size_t size) {
    key_t key = ftok(".", 'S'); // genera la chiave per la memoria condivisa
    int shmid = shmget(key, size, IPC_CREATE | 0666); // crea la memoria condivisa

    if (shmid == -1) {
        perror("shmget"); 
        exit(EXIT_FAILURE); 
    }
    
    return shmid; 
}

void init_var() {
    if (getenv("SO_PORTI")) {
        SO_PORTI = atoi(getenv("SO_PORTI")); 
        SO_NAVI = atoi(getenv("SO_NAVI")); 
        SO_LATO = atoi(getenv("SO_LATO")); 
        SO_BANCHINE = atoi(getenv("SO_BANCHINE")); 
        SO_MERCI = atoi(getenv("SO_MERCI")); 
        SO_SIZE = atoi(getenv("SO_SIZE")); 
        SO_MAX_VITA = atoi(getenv("SO_MAX_VITA")); 
        SO_MIN_VITA = atoi(getenv("SO_MIN_VITA")); 
        SO_FILL = atoi(getenv("SO_FILL")); 
    } else {
        printf("Environment variables are not initialized");
        exit(EXIT_FAILURE); 
    }
}

