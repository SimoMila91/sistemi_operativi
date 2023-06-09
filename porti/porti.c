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
#include "../utility/utility.h"
#include "porti.h"



port* port_list;
database* myData;   
int shm_port; 

good* offerList;
int shmid_offer; 


int main(int argc, char **argv) {
    if(argc != 6){
        printf("error argc");
    }
    int index = atoi(argv[1]);

    myData = shmat(atoi(argv[2]), NULL, 0); TEST_ERROR;
    int totalOffer = atoi(argv[3]); 
    int totalRequest = atoi(argv[4]); 
    int semId = atoi(argv[5]); 

    /* creo memoria condivisa per il porto */
    shm_port = createSharedMemory(sizeof(port) * 1); TEST_ERROR;
    port_list = shmat(shm_port, NULL, 0); 
    port_list->sem_inventory_id = semget(IPC_PRIVATE, 2, IPC_CREAT | 0600); TEST_ERROR;
    semctl(port_list->sem_inventory_id, 0, SETVAL, 1); /* semaforo richiesta */
    TEST_ERROR;
    semctl(port_list->sem_inventory_id, 1, SETVAL, 1); /* semaforo offerta */
    TEST_ERROR; 

    myData[index].keyPortMemory = shm_port; 

    initPort(index, totalOffer, totalRequest, semId);
    printf("finito\n");
    struct sembuf sb; 
    decreaseSem(sb, semId, 0);
    waitForZero(sb, semId, 0);
    pause(); 
    printf("pause port finished\n");
}




void initPort(int i, int totalOffer, int totalRequest, int semId) {


    struct timespec t;

    switch (i) {
        case 0:
            port_list->position.x = 0;
            port_list->position.y = 0;
            break;
        case 1: 
            port_list->position.x = 0;
            port_list->position.y = SO_LATO;
            break;

        case 2 : 
            port_list->position.x = SO_LATO;
            port_list->position.y = SO_LATO;
            break;

        case 3: 
            port_list->position.x = SO_LATO;
            port_list->position.y = 0; 
            break;
        
        default:
        clock_gettime(CLOCK_REALTIME, &t);
        port_list->position.x = (double)(t.tv_nsec%(SO_LATO*100))/ 100.0;
        clock_gettime(CLOCK_REALTIME, &t);
        port_list->position.x = (double)(t.tv_nsec%(SO_LATO*100))/ 100.0;

            break;
    }
    myData[i].position.x = port_list->position.x; 
    myData[i].position.y = port_list->position.y; 

    port_list->pid = getpid(); 
    port_list->sem_docks_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666); /* assegno il semaforo per le banchine */
    TEST_ERROR;
    initDocksSemaphore(); /* inizializzo il semaforo per le banchine */

    initializeInventory(totalOffer, totalRequest); 
} 


int isDuplicate(int numGood, int numOffer) {

    int found = 0; 
    int x; 

    for (x = 0; x < numOffer && !found; x++) {
        if (port_list->inventory.offer[x].idGood == numGood) {
            found = 1; 
        }
    }


    return found; 
} 

void initializeInventory(int totalOffer, int totalRequest) {

    int j; 
    int numGoodRequest; 
    int lifeTime;  
    int counterGoodsOffer = rand() % SO_MERCI + 1; 
    int found; /* tipo merce */ tipo merce
    int* casualAmountOffer; 
    lot* lots; 
    int shmid_lots; 

    /* inizializzo la richiesta */
    numGoodRequest = rand() % SO_MERCI + 1; 
    port_list->inventory.request.idGood = numGoodRequest; 
    port_list->inventory.request.amount = totalRequest;  
    port_list->inventory.request.requestBooked = 0; 

    /* inizializzo l'offerta */ 
    port_list->inventory.counterGoodsOffer = counterGoodsOffer; 
    casualAmountOffer = getCasualWeightPort(counterGoodsOffer, totalOffer); 

    /* creo una memoria condivisa per le offerte */
    shmid_offer = createSharedMemory(sizeof good * counterGoodsOffer); 
    offerList = shmat(shmid_offer, NULL, 0); TEST_ERROR;
    port_list->inventory.keyOffers = shmid_offer; 

    for (j = 0; j < counterGoodsOffer; j++) {

        found = rand() % SO_MERCI + 1; 
        while(j != 0 && (isDuplicate(found, counterGoodsOffer) || found == numGoodRequest)){
            found = rand() % SO_MERCI + 1; 
        } 

        offerList[j].idGood = found; 
        offerList[j].amount = casualAmountOffer[j]; 
        offerList[j].keyLots
        lifeTime = rand() % (SO_MAX_VITA + 1 - SO_MIN_VITA) + SO_MIN_VITA;
        offerList[j].life = lifeTime; 
        printf("offerta numero %d\n",  offerList[j].idGood); 

        /* creo una memoria condivisa per i lotti */
        createLoots(casualAmountOffer[j], j); 
      
               
    }
    
}


void createLoots(int amount, int index) {

    int i; 
    int maxLoots; 
    lot* lots;
    int lotSize; 
    int carico;  
    struct timespec t;

    clock_gettime(CLOCK_REALTIME, &t);
    lotSize = t.tv_nsec% SO_SIZE + 1; 
    while (lotSize > amount) {
        clock_gettime(CLOCK_REALTIME, &t);
        lotSize = t.tv_nsec% SO_SIZE + 1; 
    }

    maxLoots = amount / lotSize + 1; 
    offerList[index].maxLoots = maxLoots;
    offerList[index].keyLots = createSharedMemory(sizeof lot * maxLoots); 

    lots = shmat(offerList[index].keyLots, NULL, 0);  

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
}


int* getCasualWeightPort( int counter, int totalOffer) {
    
    int *offer = (int*)malloc(counter * sizeof(int));
        int sum = 0;
        int i;
        srand(time(NULL));
        for(i=0; i<counter; i++){
            offer[i] = rand()%totalOffer+1;
            sum += offer[i];
            

        }

        for(i=0; i<counter; i++){
            offer[i] = (totalOffer * offer[i])/ sum;
            /*resto -= offer[i];*/
            if(offer[i] == 0) offer[i]++;
        

        }

        
        return offer;  

}

void initDocksSemaphore() {

    if (port_list->sem_docks_id == -1) {
        perror("semget"); 
        exit(EXIT_FAILURE); 
    }

    if (semctl(port_list->sem_docks_id, 0, SETVAL, (rand() % SO_BANCHINE + 1)) == -1) {
        perror("semctl"); 
        exit(EXIT_FAILURE); 
    }
}

int createSharedMemory(size_t size) {

    int shmid; 

    shmid = shmget(IPC_PRIVATE, size, IPC_CREAT| 0600); 

    if (shmid == -1) {
        perror("shmget"); 
        exit(EXIT_FAILURE); 
    }
    
    return shmid; 
}