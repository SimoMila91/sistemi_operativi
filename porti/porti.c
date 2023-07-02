#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "../macro/macro.h"
#include "../master/master.h"
#include "porti.h"



int SO_LATO;  
int SO_BANCHINE; 
int SO_MERCI; 
int SO_SIZE; 
int SO_MAX_VITA; 
int SO_MIN_VITA; 
int SO_FILL; 
port* portList;
database* myData;  
int shmid_port; 


int main(int argc, char **argv) {
    if(argc != 6){
        printf("error argc");
    }
    int index = atoi(argv[1]);
    myData = (database*)shmat(atoi(argv[2]), NULL, 0); 
    double totalOffer = atoi(argv[3]); 
    double totalRequest = atoi(argv[4]); 
    int semId = atoi(argv[5]); 

    // creo la memoria condivisa per il porto 
    shmid_port = createSharedMemory(sizeof(port) * 1); 
    portList = shmat(shmid_port, NULL, 0); 
    portList->sem_inventory_id = semget(IPC_PRIVATE, 2, IPC_CREAT | 0600);
    semctl(portList->sem_inventory_id, 0, SETVAL, 1); //richiesta
    TEST_ERROR;
    semctl(portList->sem_inventory_id, 1, SETVAL, 1); //offerta
    TEST_ERROR; 

    myData[index].keyPortMemory = shmid_port; 
   
    initPort(index, totalOffer, totalRequest, semId);
  
}




void initPort(int i, double totalOffer, double totalRequest, int semId) {


    struct timespec t;

    portList->position = (coordinate*)malloc(sizeof(coordinate*)); 
    myData->position = (coordinate*)malloc(sizeof(coordinate*));

    switch (i) {
        case 0:
            portList->position->x = 0;
            portList->position->y = 0;
            break;
        case 1: 
            portList->position->x = 0;
            portList->position->y = SO_LATO;
            break;

        case 2 : 
            portList->position->x = SO_LATO;
            portList->position->y = SO_LATO;
            break;

        case 3: 
            portList->position->x = SO_LATO;
            portList->position->y = 0; 
            break;
        
        default:
        clock_gettime(CLOCK_REALTIME, &t);
        portList->position->x = (double)(t.tv_nsec%(SO_LATO*100))/ 100.0;
        clock_gettime(CLOCK_REALTIME, &t);
        portList->position->x = (double)(t.tv_nsec%(SO_LATO*100))/ 100.0;

            break;
    }

    myData[i].position = portList->position;
    
    portList->pid = getpid(); 
    portList->sem_docks_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666); // assegno semaforo 
    initDocksSemaphore(); // inizializzo il semaforo
    initializeInventory(totalOffer, totalRequest); 

    struct sembuf sb; 
    sb.sem_num = 0; 
    sb.sem_op = -1; 
    sb.sem_flg = 0; 

    if (semop(semId, &sb, 1) == -1) {   /////semId da inizializzare??? 
        perror("semop porto"); 
        exit(EXIT_FAILURE); 
    }

    sb.sem_num = 0; 
    sb.sem_op = 0; 
    sb.sem_flg = 0; 
    
    if (semop(semId, &sb, 1) == -1) {
        perror("semop porto"); 
        exit(EXIT_FAILURE); 
    }
    
} 


int isDuplicate(int numGood, int numOffer) {

    int found = 0; 
    int x; 

    for (x = 0; x < numOffer && !found; x++) {
        if (portList->inventory.offer[x].idGood == numGood) {
            found = 1; 
        }
    }

    return found; 
} 

void initializeInventory(double totalOffer, double totalRequest) {

    int j; 
    int numGoodRequest; 
    int lifeTime;  
    int counterGoodsOffer = rand() % SO_MERCI + 1; // ad esempio 3 tipi di merce 
    int found; // tipo merce
    double casualAmountOffer[counterGoodsOffer]; 

    // inizializzo la richiesta 
    numGoodRequest = rand() % SO_MERCI + 1; 
    portList->inventory.request.idGood = numGoodRequest; 
    portList->inventory.request.amount = totalRequest;  
    portList->inventory.request.requestBooked = 0; 

    //inizializzo l'offerta 
    
    portList->inventory.counterGoodsOffer = counterGoodsOffer;
 
    getCasualWeightPort(casualAmountOffer, counterGoodsOffer, totalOffer); 

    portList->inventory.offer = (good*)malloc(counterGoodsOffer * sizeof(good)); 

    for (j = 0; j < counterGoodsOffer; j++) {

        found = rand() % SO_MERCI + 1; 
        while(j != 0 && (isDuplicate(found, counterGoodsOffer) || found == numGoodRequest)){
            found = rand() % SO_MERCI + 1; 
        } 

        portList->inventory.offer[j].idGood = found; 
        portList->inventory.offer[j].amount = casualAmountOffer[j]; 
        
        portList->inventory.offer[j].lots = createLoots(casualAmountOffer[j], j); 
        lifeTime = rand() % (SO_MAX_VITA + 1 - SO_MIN_VITA) + SO_MIN_VITA;
        portList->inventory.offer[j].life = lifeTime;          
    }
}


lot* createLoots(double amount, int index) {

    int i; 
    int maxLoots; 
    lot* lots;
    double lotSize; 
    double carico; //////aggiunto rachy 

    lotSize = rand() % SO_SIZE + 1; 
    while (lotSize > amount) {
        lotSize = rand() % SO_SIZE + 1; 
    }

    maxLoots = amount / lotSize + 1; 
    portList->inventory.offer[index].maxLoots = maxLoots;
    lots = malloc(maxLoots * sizeof(lot)); 

    for (i = 0; i < maxLoots-1; i++) {
        lots[i].value = lotSize; 
        lots[i].available = 1; 
        lots[i].type = 0; 
        carico -= lotSize; 
        lots[i].id_ship = -1;
    }

    lots[maxLoots-1].value = carico; 
    lots[maxLoots-1].available = 1; 
    lots[maxLoots-1].type = 0; 

    return lots; 
}


void getCasualWeightPort(double offer[], int counter, double totalOffer) {
    
    int i; 
    srand(time(NULL)); 
    double sum = 0.0; 

    for (i = 0; i < counter-1; i++) {
        offer[i] = (double)rand() / RAND_MAX * (totalOffer - sum); 
        sum += offer[i]; 
    }

    offer[counter] = totalOffer - sum; 

}

void initDocksSemaphore() {

    if (portList->sem_docks_id == -1) {
        perror("semget"); 
        exit(EXIT_FAILURE); 
    }

    if (semctl(portList->sem_docks_id, 0, SETVAL, (rand() % SO_BANCHINE + 1)) == -1) {
        perror("semctl"); 
        exit(EXIT_FAILURE); 
    }
}

int createSharedMemory(size_t size) {

    int shmid; 

    key_t key = ftok(".", 'S'); // genera la chiave per la memoria condivisa
    shmid = shmget(key, size, IPC_CREAT| 0600); // crea la memoria condivisa

    if (shmid == -1) {
        perror("shmget"); 
        exit(EXIT_FAILURE); 
    }
    
    return shmid; 
}