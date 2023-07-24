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
    dumpSimulation(1); 
}

void alarmHandler(int signum) {
    printf("%d", signum); /*da cancellare*/
      
    if (daysRemains > 0) {
        // dump giornaliero 
        printf("[ REPORT PROVVISORIO ]\n\n"); 
        pause(); 
        /* dumpSimulation(0); */
    } 
    daysRemains--; 
}

void dumpSimulation(int type) { 

    int i; 
    int j; 
    int k; 
    port* currentPort;
    good* currentOffer; 
    lot* currentLot;  
    int idGood; 
    int totalGoods[SO_MERCI+1][5]; 
    int reportPorts[SO_PORTI][6]; 
    int shipWithCargo = 0; 
    int shipWithoutCargo = 0; 
    int shipToPort = 0; 
    int semval; 
    /* final report variables */
    int goodsReport[SO_MERCI+1][5]; 
    int maxOfferPort = [SO_MERCI+1][2];
    int maxRequestPort = [SO_MERCI+1][2];  

    /**
     * ! totalGoods 
     * * 0 = presente in porto
     * * 1 = presente su nave
     * * 2 = consegnata ad un porto
     * * 3 = scaduta in porto  
     * * 4 = scaduta in nave 
    */

     /**
     * ! reportPorts 
     * * 0 = pid del porto
     * * 1 = presente in porto
     * * 2 = spedita dal porto
     * * 3 = ricevuta in porto 
     * * 4 = banchine occupate
     * * 5 = banchine totali
    */
   
    /**
     * ! goodsReport 
     * * 0 = quantità totale generata
     * * 1 = rimasta ferma in un porto
     * * 2 = scaduta in porto 
     * * 3 = scaduta in nave
     * * 4 = consegnata
    */

    /**
     * ! maxOfferPort && maxRequestPort 
     * * 0 = quantità della merce
     * * 1 = pid porto 
     * 
     * ? incremento di 1 il valore SO_MERCI in modo da far corrispondere l'idGood alla rispettiva riga della matrice
    */
   

    totalGoods = initializeMatrix(totalGoods, SO_MERCI); 
    reportPorts = initializeMatrix(reportPorts, SO_PORTI); 

    if (type) {
        goodsReport = initializeMatrix(goodsReport, SO_MERCI); 
    }

    /* totalGoods ports and ships */

    for (i = 0; i < SO_PORTI; i++) {

        currentPort = shmat(portList[i].keyPortMemory, NULL, 1); 

        reportPorts[i][0] = currentPort->pid; 
        reportPorts[i][3] = currentPort->inventory.request.amount -  currentPort->inventory.request.remains;
        semval = semctl(currentPort->sem_docks_id, 0 GETVAL); 
        reportPorts[i][4] = (currentPort->totalDocks - semval); 
        reportPorts[i][5] = currentPort->totalDocks; 

        if (type) {
            if (currentPort->inventory.request.amount > maxRequestPort[currentPort->inventory.request.idGood][0]) {
                maxRequestPort[currentPort->inventory.request.idGood][0] = currentPort->inventory.request.amount; 
                maxRequestPort[currentPort->inventory.request.idGood][1] = currentPort->pid; 
            }
        }

        currentOffer = shmat(currentPort->inventory.keyOffers, NULL, 1); 

        int difference = currentPort->inventory.request.amount - currentPort->inventory.request.remains; 
        totalGoods[currentPort->inventory.request.idGood][2] += difference; 

        for (j = 0; j < currentPort->inventory.counterGoodsOffer; j++) {

            if (type) {
                goodsReport[currentOffer[j].idGood][0] += currentOffer[j].amount; 

                if(currentOffer[j].amount > maxOfferPort[currentOffer[j].idGood][0]) {
                    maxOfferPort[currentOffer[j].idGood][0] = currentOffer[j].amount; 
                    maxOfferPort[currentOffer[j].idGood][1] = currentPort->pid; 
                }

            }

            currentLot = shmat(currentOffer[j].keyLots, NULL, 1); 

            for (k = 0; k < currentOffer[j].maxLoots; k++) {


                if (currentLot->available) {
                    reportPorts[i][1] += currentLot->value; 
                } else {
                    reportPorts[i][2] += currentLot->value; 
                }

                idGood = currentLot[k].idGood; 
                switch (currentLot[k].status){
                    case 0:
                        if (currentLot[k].life > (SO_DAYS - daysRemains)) {
                            totalGoods[idGood][0] += currentLot[k].value; 
                        } else {
                            if (type) {
                                goodsReport[currentLot[k].idGood][2] += currentLot[k].value; 
                            }
                            totalGoods[idGood][3] += currentLot[k].value; 
                        }
                        break;
                    case 1: 
                        if (currentLot[k].life > (SO_DAYS - daysRemains)) {
                            totalGoods[idGood][1] += currentLot[k].value; 
                        } else {
                            if (type) {
                                goodsReport[currentLot[k].idGood][3] += currentLot[k].value; 
                            }
                            totalGoods[idGood][4] += currentLot[k].value; 
                        }
                        break; 
                    case 2: 
                        totalGoods[idGood][2] += currentLot[k].value; 
                        if (type) {
                            goodsReport[currentLot[k].idGood][4] += currentLot[k].value; 
                        }
                        break; 
                    default:
                        break;
                }
                
            }
            shmid(currentLot); TEST_ERROR; 
        } 
        shmid(currentPort); TEST_ERROR; 
        shmid(currentOffer); TEST_ERROR;   
    } 

    for (i = 0; i < SO_NAVI; i++) {
        
        if (shipList[i].statusPosition == 0 && shipList[i].statusCargo == 0) {
            shipWithoutCargo++; 
        } 

        if (shipList[i].statusPosition == 0 && shipList[i].statusCargo == 1) {
            shipWithCargo++; 
        }

        if (shipList[i].statusPosition == 1) {
            shipToPort++; 
        }
    }
}

void initializeMatrix(int matrix[][], int length) {
    int i; 
    int j; 
    for (i = 0; i < length; i++) {
        for (j = 0; j < 5; j++) {
            matrix[i][j] = 0; 
        }
    }
    return matrix; 
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



