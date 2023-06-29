#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include "../master/master.h"
#include "../macro/macro.h"
#include "porti.h"


double SO_LATO;  
int SO_BANCHINE; 
int SO_MERCI; 
int SO_SIZE; 
int SO_MAX_VITA; 
int SO_MIN_VITA; 
int SO_FILL; 
port portList;
database* myData;  
int shmid_port; 


int main(int argc, char **argv) {

    int index = atoi(argv[1]);
    myData = (database*)shmat(atoi(argv[2]), NULL, 0); 
    double totalOffer = atoi(argv[3]); 
    double totalRequest = atoi(argv[4]); 
    int semId = atoi(argv[5]); 
    init_var();

    // creo la memoria condivisa per il porto 
    shmid_port = createSharedMemory(sizeof(port) * 1); 
    portList = (port)shmat(shmid_database, NULL, 0); 

    myData[index].keyPortMemory = shmid_port; 
   
    initPort(index, portList, totalOffer, totalRequest, semId);
  
}


void init_var() {
    if (getenv("SO_PORTI")) {
        SO_LATO = atoi(getenv("SO_LATO")); 
        SO_BANCHINE = atoi(getenv("SO_BANCHINE")); 
        SO_MERCI = atoi(getenv("SO_MERCI")); 
        SO_SIZE = atoi(getenv("SO_SIZE")); 
        SO_MAX_VITA = atoi(getenv("SO_MAX_VITA")); 
        SO_MIN_VITA = atoi(getenv("SO_MIN_VITA")); 
    } else {
        printf("Environment variables are not initialized");
        exit(EXIT_FAILURE); 
    }
}



void initPort(int i, port portList, double totalOffer, double totalRequest, int semId) {

    int numRequest; 
    int numOffer; 

    portList->position = (coordinate*)malloc(sizeof(coordinate)); 
    myData.position = (coordinate*)malloc(sizeof(coordinate));

    switch (i) {
        case 0:
            portList.position->x = 0;
            portList.position->y = 0;
            break;
        case 1: 
            portList.position->x = 0;
            portList.position->y = SO_LATO;
            break;

        case 2 : 
            portList.position->x = SO_LATO;
            portList.position->y = SO_LATO;
            break;

        case 3: 
            portList.position->x = SO_LATO;
            portList.position->y = 0; 
            break;
        
        default:
            portList.position->x = (rand()%(SO_LATO + 1));
            portList.position->y = (rand()%(SO_LATO + 1));

            break;
    }

    myData[index].position = portList.position;
    
    portList->pid = getpid(); 
    portList->sem_docks_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666); // assegno semaforo 
    initDocksSemaphore(portList); // inizializzo il semaforo
    initializeInventory(portList, totalOffer, totalRequest); 

    struct sembuf sb; 
    sb.sem_num = 0; 
    sb.sem_op = -1; 
    sb.sem_flg = 0; 

    if (semop(semId, &sb, 1) == -1) {   /////semId da inizializzare??? 
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
    
} 


int isDuplicate(int numGood, int numOffer, port* portList) {

    int found = 0; 
    int x; 

    for (x = 0; x < numOffer && !found; x++) {
        if (portList.inventory.offer[x].idGood == numGood) {
            found = 1; 
        }
    }

    return found; 
} 

void initializeInventory(port portList, double totalOffer, double totalRequest) {

    int j; 
    int numGoodRequest; 
    int lifeTime;  
    int counterGoodsOffer; 
    int found; // tipo merce

    // inizializzo la richiesta 
    numGoodRequest = rand() % SO_MERCI + 1; 
    portList->inventory.request = (good*)malloc(1 * sizeof(good)); 
    portList->inventory.request[0].idGood = numGoodRequest; 
    portList->inventory.request[0].amount = totalRequest;  // [0] da mettere? DIO ESISTE? per Stefano
    portList.inventory.request->requestBooked = 0; 

    //inizializzo l'offerta 
    counterGoodsOffer = rand() % SO_MERCI + 1; // ad esempio 3 tipi di merce 
    double casualAmountOffer[counterGoodsOffer]; 
    getCasualWeight(casualAmountOffer, counterGoodsOffer, totalOffer); 

    portList->inventory.offer = (good*)malloc(counterGoodsOffer * sizeof(good)); 

    for (j = 0; j < counterGoodsOffer; j++) {

        found = rand() % SO_MERCI + 1; 
        while((j != 0 && isDuplicate(found, counterGoodsOffer, portList)) || found == numGoodRequest) {
            found = rand() % SO_MERCI + 1; 
        } 

        portList->inventory.offer[j].idGood = found; 
        portList->inventory.offer[j].amount = casualAmountOffer[j]; 
        
        portList->inventory.offer[j].lootSize = createLoot(casualAmountOffer[j]); 
        lifeTime = rand() % (SO_MAX_VITA + 1 - SO_MIN_VITA) + SO_MIN_VITA;
        portList->inventory.offer[j].life = lifeTime;          
    }
}

lot* createLoots(double amount) {

    int i = 0; 
    int maxLoots; 
    lot* lots;
    double lotSize; 
    double carico; //////aggiunto rachy 

    lotSize = rand() % SO_SIZE + 1; 
    while (lotSize > amount) {
        lotSize = rand() % SO_SIZE + 1; 
    }

    maxLoots = amount / lotSize + 1; 
    lots = (double*)malloc(maxLoots * sizeof(lot)); 

    for (i; i < maxLoots-1; i++) {
        lots[i].value = lotSize; 
        lots[i].available = 1; 
        lots[i].type = 0; 
        lots[i].booked = 1; 
        carico -= lotSize; 
    }

    lots[maxLoots-1].value = carico; 
    lots[maxLoots-1].available = 1; 
    lots[maxLoots-1].type = 0; 
    lots[maxLoots-1].booked = 1; 

    return lots; 
}


void getCasualWeight(double* offer, int counter, double totalOffer) {
    
    int i; 
    srand(time(NULL)); 
    double goodSum = 0.0; 

    for (i = 0; i < counter-1; i++) {
        offer[i] = (double)rand() / RAND_MAX * (totalOffer - sum); 
        sum += offer[i]; 
    }

    offer[counter] = totalOffer - sum; 

}

void initDocksSemaphore(port portList) {

    if (portList.sem_docks_id == -1) {
        perror("semget"); 
        exit(EXIT_FAILURE); 
    }

    if (semctl(portList.sem_docks_id, 0, SETVAL, (rand() % SO_BANCHINE + 1)) == -1) {
        perror("semctl"); 
        exit(EXIT_FAILURE); 
    }
}