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
port* portList;



int main(int argc, char **argv) {

  int index = atoi(argv[1]);
  portList = (port*)shmat(atoi(argv[2]), NULL, 0); 
  double totalOffer = atoi(argv[3]); 
  double totalRequest = atoi(argv[4]); 
  int semId = atoi(argv[5]); 
 

  init_var();
  initPort(index, portList[index], totalOffer, totalRequest, semId);
  
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



void initPort(int i, port* portList, double totalOffer, double totalRequest, int semId) {

    int numRequest; 
    int numOffer; 

    portList->position = (coordinate*)malloc(sizeof(coordinate)); 

    switch (i) {
        case 0:
            portList[0].position->x = 0;
            portList[0].position->y = 0;
            break;
        case 1: 
            portList[1].position->x = 0;
            portList[1].position->y = SO_LATO;
            break;

        case 2 : 
            portList[2].position->x = SO_LATO;
            portList[2].position->y = SO_LATO;
            break;

        case 3: 
            portList[3].position->x = SO_LATO;
            portList[3].position->y = 0; 
            break;
        
        default:
            portList[i].position->x = (rand()%(SO_LATO + 1));
            portList[i].position->y = (rand()%(SO_LATO + 1));

            break;
    }
    portList->pid = getpid(); 
    portList->sem_docks_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666); // assegno semaforo 
    initDocksSemaphore(portList); // inizializzo il semaforo
    initializeInventory(portList, totalOffer, totalRequest); 

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

void initializeInventory(port* portList, double totalOffer, double totalRequest) {

    int j; 
    int numGoodRequest; 
    int lifeTime;  
    int counterGoodsOffer; 
    int found; // tipo merce

    // inizializzo la richiesta 
    numGoodRequest = rand() % SO_MERCI + 1; 
    portList->inventory.request = (good*)malloc(1 * sizeof(good)); 
    portList->inventory.request[0].idGood = numGoodRequest; 
    portList->inventory.request[0].amount = totalRequest; 

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
        carico -= lotSize; 
    }

    lots[maxLoots-1].value = carico; 
    lots[maxLoots-1].available = 1; 
    lots[maxLoots-1].type = 0; 

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

void initDocksSemaphore(port* portList) {

    if (portList.sem_docks_id == -1) {
        perror("semget"); 
        exit(EXIT_FAILURE); 
    }

    if (semctl(portList.sem_docks_id, 0, SETVAL, (rand() % SO_BANCHINE + 1)) == -1) {
        perror("semctl"); 
        exit(EXIT_FAILURE); 
    }
}