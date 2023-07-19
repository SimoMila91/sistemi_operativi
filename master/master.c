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
#include "master.h"




// port's variables 
int shmid_port; 
database* portList;

// ship's variables 
int shmid_ship;
ship* shipList;

int daysRemains; 

int main() {

    int i;
    char* args[7];
    char idx_port[3*sizeof(int)+1];
    char idx_ship[3*sizeof(int)+1];
    char shmid_port_str[3*sizeof(int)+1]; 
    char shmid_ship_str[3*sizeof(int)+1];
    char valueTotalOffer[3*sizeof(int)+1]; 
    char valueTotalRequest[3*sizeof(int)+1]; 
    char keySemMaster[3*sizeof(int)+1]; 
    char name_file[10];
    int* offerArray; 
    int* requestArray; 
    int semStartSimulation; 
    daysRemains = SO_DAYS;
   
    semStartSimulation =  semget(IPC_PRIVATE, 1, IPC_CREAT | 0666); // assegno semaforo 
    TEST_ERROR;
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
    args[6] = NULL;
   
    offerArray = getCasualWeight();
    requestArray = getCasualWeight();  

    sprintf(name_file, "porti");
    args[0] = name_file;
   
    int shmid_port = shmget(IPC_PRIVATE, sizeof(database) * SO_PORTI, 0600|IPC_CREAT); TEST_ERROR;
    portList = shmat(shmid_port, NULL, 0); TEST_ERROR; 
    sprintf(shmid_port_str, "%d", shmid_port); 
    args[2] = shmid_port_str;

    

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
            sprintf(valueTotalOffer, "%d", offerArray[i]); 
            sprintf(valueTotalRequest, "%d", requestArray[i]); 
            args[1] = idx_port; 
            args[3] = valueTotalOffer; 
            args[4] = valueTotalRequest;  
            execv("./porti/porti", args);
            TEST_ERROR;
            exit(EXIT_FAILURE);
        default:
            //padre 
            break;
        }
    }

    args[2] = shmid_port_str;
    args[4]= keySemMaster;
    //args[5] = NULL; 
    sprintf(name_file, "navi");
    args[0] = name_file;
    shmid_ship = shmget(IPC_PRIVATE, sizeof(ship) * SO_NAVI, 0600|IPC_CREAT); TEST_ERROR; 
    shipList = shmat(shmid_ship, NULL, 0); TEST_ERROR;
    sprintf(shmid_ship_str, "%d", shmid_ship);
    args[3] = shmid_ship_str;

    for(i = 0; i < SO_NAVI; i++) {
        pid_t pid = fork(); TEST_ERROR;
        switch (pid)
        {
            case -1:
                printf("error in fork of ship's process\n" );
                exit(EXIT_FAILURE);
                break;
            case 0: 
                sprintf(idx_ship, "%d", i);
                args[1] = idx_ship; TEST_ERROR;
                execv("./navi/navi", args);
                TEST_ERROR;
                exit(EXIT_FAILURE);
                break;
            default:
                //padre 
                break;
        }
    }
   
    struct sembuf sb;     
    decreaseSem(sb, semStartSimulation, 0);
    waitForZero(sb, semStartSimulation, 0);

    // inizializzazione simulazione 
    while(daysRemains > 0) { 
        alarm(1); 
        pause();  // franco pippo 
    }
    printf("fine master\n"); 
    // REPORT FINALE 
}

void alarmHandler(int signum) {
    printf("%d", signum); /*da cancellare*/
    
    
    if (daysRemains > 0) {
        // dump giornaliero 
        printf("[ REPORT PROVVISORIO ]\n\n"); 
        pause(); 
        /* dumpSimulation(); */
    } 
    daysRemains--; 
}

void dumpSimulation() {

    int i; 
    int j; 
    int k; 
    port* currentPort;
    good* currentOffer; 
    lot* currentLot; 
    int totalGoods[SO_MERCI][5]; 
    /*
    0 = presente in porto 
    1 = presente su nave
    2 = consegnata ad un porto 
    3 = scaduta in porto 
    4 = scaduta in nave 
    */

    for (i = 0; i < SO_MERCI; i++) {
        for (j = 0; j < 5; j++) {
            totalGoods[i][j] = 0; 
        }
    }

    /* totalGoods ports */

    for (i = 0; i < SO_PORTI; i++) {
        currentPort = shmat(portList[i].keyPortMemory, NULL, 1); 
        currentOffer = shmat(currentPort->inventory.keyOffers, NULL, 1); 

        int difference = currentPort->inventory.request.amount - currentPort->inventory.request.remains; 
        totalGoods[currentPort->inventory.request.idGood][2] += difference; 

        for (j = 0; j < currentPort->inventory.counterGoodsOffer; j++) {
            currentLot = shmat(currentOffer[j].keyLots, NULL, 1); 

            for (k = 0; k < currentOffer[j].maxLoots; k++) {
                int idGood = currentOffer[j].idGood; 

                if (currentLot[k].available && currentOffer[j].life > (SO_DAYS - daysRemains)) {
                    totalGoods[idGood][0] += currentLot[k].value; 
                } 

                if (currentOffer[j].life < (SO_DAYS - daysRemains) && currentLot[k].available) {
                    totalGoods[idGood][3] += currentLot[k].value; 
                }
                
            }
            shmid(currentLot); TEST_ERROR; 
        } 
        shmid(currentPort); TEST_ERROR; 
        shmid(currentOffer); TEST_ERROR;   
    } 

    /* totalGoods ships */

    for (i = 0; i < SO_NAVI; i++) {
    
    }

}


int checkEconomy() {
    
    int i; 
    int j; 
    int k;
    int foundRequest = 0; 
    int res; 
    int l; 
    port* currentPort;  
    good* offerList; 
    port* requestPort; 
    lot* lot; 
    struct sembuf sb; 
    bzero(&sb, sizeof(struct sembuf ));
    
    for (i = 0; i < SO_PORTI && !res; i++) {
        requestPort = shmat(portList[i].keyPortMemory, NULL, 0); 
        decreaseSem(sb, requestPort->sem_inventory_id, 0 );
        if (requestPort->inventory.request.remains > 0) {
            foundRequest = requestPort->inventory.request.idGood; 
        }   
        increaseSem(sb, requestPort->sem_inventory_id, 0);
        int found = 0;  
        for (j = 0; j < SO_PORTI && !found; j++) {
            currentPort = shmat(portList[i].keyPortMemory, NULL, 1);  
            offerList = shmat(currentPort->inventory.keyOffers, NULL, 1); 

            for(k = 0; k < currentPort->inventory.counterGoodsOffer &&  !found; k++ ) {
                lot = shmat(offerList[k].keyLots, NULL, 1); 


                for(l = 0; l < offerList->maxLoots && !found; l++) {
                    decreaseSem(sb, offerList[k].semLot, l); 

                    if (lot[l].available && offerList[k].idGood == foundRequest) {
                        found = 1; 
                    }   
                    increaseSem(sb, offerList[k].semLot, l); 
                }
                shmdt(lot); TEST_ERROR; 

                
            }
            increaseSem(sb, requestPort->sem_inventory_id, 1);
            shmdt(currentPort); TEST_ERROR; 
            shmdt(offerList); 
        }
        
        res = found;
        shmdt(requestPort); TEST_ERROR; 
    }
    if (shmdt(requestPort) == -1) {
        perror("shmdt: ship -> checkEconomy"); 
        exit(EXIT_FAILURE); 
    }

    return res;  
}

int* getCasualWeight() {


    int *offer = (int*)malloc(SO_PORTI * sizeof(int));
    int sum = 0;
    int i;

    srand(time(NULL));
    for(i=0; i<SO_PORTI; i++){
        offer[i] = rand()%SO_FILL+1;
        sum += offer[i];
        

    }

    for(i=0; i<SO_PORTI; i++){
        offer[i] = (SO_FILL * offer[i])/ sum;
        /*resto -= offer[i];*/
        if(offer[i] == 0) offer[i]++;
    

    }

    
    return offer; 
    

}



