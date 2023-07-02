#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include "master.h"
#include "../macro/macro.h"

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
database* portList;

// ship's variables 
int shmid_ship;
ship* shipList;

int daysRemains; 

int main(int argc, char **argv) {

    init_var();  // initialize of variables 
    int i;
    char* args[6];
    char* idx_port[3*sizeof(int)+1];
    char* idx_ship[3*sizeof(int)+1];
    char* shmid_port_str[3*sizeof(int)+1]; 
    char* shmid_ship_str[3*sizeof(int)+1];
    char* valueTotalOffer[3*sizeof(double)+1]; 
    char* valueTotalRequest[3*sizeof(double)+1]; 
    char* keySemMaster[3*sizeof(int)+1]; 
    double offerArray[SO_PORTI]; 
    double requestArray[SO_PORTI]; 
    int memoryKeys[SO_PORTI]; 
    int memoryKeysLength = sizeof(memoryKeys) / sizeof(int); 
    int semStartSimulation =  semget(IPC_PRIVATE, 1, IPC_CREAT | 0666); // assegno semaforo
    daysRemains = SO_DAYS; 
    signal(SIGALRM, alarmHandler);

    if (semStartSimulation == -1) {
        perror("semget"); 
        exit(EXIT_FAILURE); 
    }

    if (semctl(semStartSimulation, 0, SETVAL, (SO_PORTI + SO_NAVI + 1)) == -1) {
        perror("semctl"); 
        exit(EXIT_FAILURE); 
    }

    sprintf(keySemMaster, "%d", semStartSimulation);
    args[5]  = keySemMaster; 

    getCasualWeight(offerArray);
    getCasualWeight(requestArray);  

    args[0] = "porti";

   
    int shmid_port = createSharedMemory(sizeof(database) * SO_PORTI); 
    portList = (port*)shmat(shmid_port, NULL, 0); 
    sprintf(shmid_port_str, "%d", shmid_port); 
    args[2] = shmid_ship_str;
   
    

    for(i=0; i < SO_PORTI; i++) {
        pid_t pid = fork(); 
        switch (pid)
        {
        case -1:
            printf("error in fork of port's process\n" );
            exit(EXIT_FAILURE);
            break;
        case 0: 
            sprintf(idx_port, "%d", i);
            sprintf(valueTotalOffer, "%.2f", offerArray[i]); 
            sprintf(valueTotalRequest, "%.2f", requestArray[i]); 
            args[1] = idx_port; 
            args[3] = valueTotalOffer; 
            args[4] = valueTotalRequest;  
            execv("../porti/porti.c", args);
            TEST ERROR;
            exit(EXIT_FAILURE);
        default:
            //padre 
            break;
        }
    }



    args[0] = "navi";

    shmid_nave = createSharedMemory(sizeof(port) * SO_NAVI); 
    shipList = (port*)shmat(shmid_ship, NULL, 0); 
    sprintf(shmid_ship_str, "%d", shmid_ship);
    args[3] = shmid_ship_str;

    for(i = 0; i < SO_NAVI; i++) {
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

    struct sembuf sb; 
    sb.sem_num = 0; 
    sb.sem_op = -1; 
    sb.sem_flg = 0; 

    if (semop(semId, &sb, 1) == -1) {
        perror("semop porto"); 
        exit("EXIT FAILURE"); 
    }

    sb.sem_num = 0; 
    sb.sem_op = 0; 
    sb.sem_flg = 0; 
    
    if (semop(semId, &sb, 1) == -1) {
        perror("semop porto"); 
        exit("EXIT FAILURE"); 
    }


    // inizializzazione simulazione 
    while(daysRemains > 0) { 
        alarm(1); 
        pause();  // franco pippo 
    }

    // REPORT FINALE 
}

void alarmHandler(int signum) {

    daysRemains--; 
    
    if (daysRemains > 0) {
        // REPORT GIORNALIERI 
    } 

}

int checkEconomy() {
    
    int i; 
    int j; 
    int k;
    int foundRequest = 0; 
    int res; 
    good* currentOffer; 
    port portList; 
    struct sembuf sb; 
    bzero(&sb, sizeof(struct sembuf ));
    
    for (i = 0; i < SO_PORTI && !res; i++) {
        portList = (port)shmat(db[i].keyPortMemory, NULL, 0); 
        decreaseSem(sb, portList.sem_inventory_id, 0 );
        if (portList.inventory.request.lots->available) {
            foundRequest = portList.inventory.request.idGood; 
        }   
        increaseSem(sb, portList.sem_inventory_id, 0);
        int found = 0;  
        for (j = 0; j < SO_PORTI && !found; j++) {
            currentOffer = (port)shmat(db[i].keyPortMemory, NULL, 1);  
            decreaseSem(sb, portList.sem_inventory_id, 0 );
            for(k = 0; k < portList.inventory.counterGoodsOffer &&  !found; k++ ){
                for(l = 0; l < portList.inventory.offer->maxLoots && !found; l++){
                    if (currentOffer[k].lots[l].available && currentOffer->idGood == foundRequest) {
                        found = 1; 
                    }   
                }
                
            }
            increaseSem(sb, portList.sem_inventory_id, 1);
            shmdt(currentOffer); TEST_ERROR; 
        }
        
        res = found;
        shmdt(portList); TEST_ERROR; 
    }
    if (shmdt(portList) == -1) {
        perror("shmdt: ship -> checkEconomy"); 
        exit(EXIT_FAILURE); 
    }

    return res;  
}

void getCasualWeight(double* offer) {
    
    int i; 
    srand(time(NULL)); 
    double goodSum = 0.0; 

    for (i = 0; i < SO_PORTI-1; i++) {
        offer[i] = (double)rand() / RAND_MAX * (SO_FILL - sum); 
        sum += offer[i]; 
    }

    offer[SO_PORTI -1] = SO_FILL - sum; 

}

int createSharedMemory(size_t size) {

    int shmid; 

    key_t key = ftok(".", 'S'); // genera la chiave per la memoria condivisa
    shmid = shmget(key, size, IPC_CREAT E | 0666); // crea la memoria condivisa

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