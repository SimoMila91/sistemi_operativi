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
#include <sys/msg.h>
#include "../macro/macro.h"
#include "../utility/utility.h"
#include "master.h"




// port's variables 
int shmid_port; 
database* portList;

// ship's variables 
int shmid_ship;
ship* shipList;

int* daysRemains; 

int main() {

    int i;
    char* args[9];
    char idx_port[3*sizeof(int)+1];
    char idx_ship[3*sizeof(int)+1];
    char shmid_port_str[3*sizeof(int)+1]; 
    char shmid_ship_str[3*sizeof(int)+1];
    char valueTotalOffer[3*sizeof(int)+1]; 
    char valueTotalRequest[3*sizeof(int)+1]; 
    char keySemMaster[3*sizeof(int)+1]; 
    char name_file[10];
    char shimd_days_str[3*sizeof(int)+1];
    char msg_id_str[3*sizeof(int)+1];
    int* offerArray; 
    int* requestArray; 
    int semStartSimulation; 
    int shmid_day = shmget(IPC_PRIVATE, sizeof(int), 0600|IPC_CREAT); TEST_ERROR;
    int msg_id;
    daysRemains = shmat(shmid_day, NULL, 0);
    *daysRemains = SO_DAYS;
    msg_id = msgget(getpid(), IPC_CREAT | IPC_EXCL | 0600); TEST_ERROR;
    sprintf(msg_id_str,"%d", msg_id);
    
    sprintf(shimd_days_str, "%d", shmid_day);
    semStartSimulation =  semget(IPC_PRIVATE, 1, IPC_CREAT | 0666); // assegno semaforo
    TEST_ERROR;
    signal(SIGALRM, alarmHandler);

    semctl(semStartSimulation, 0, SETVAL, (SO_PORTI + SO_NAVI + 1)); TEST_ERROR;
 
    sprintf(keySemMaster, "%d", semStartSimulation);
    args[5]  = keySemMaster; 
    args[6] = shimd_days_str;
    args[7] = NULL;
   
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
    args[7] = msg_id_str;
    args[8] = NULL;
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
    printf("PROVA\n");
    // inizializzazione simulazione 
    while(*daysRemains > 0) { 

        alarm(1); 
        pause();  // franco pippo 
    }
    printf("fine master\n"); 
    // REPORT FINALE 
    dumpSimulation(1); 
    killProcess();
}

void killProcess(){
    int i; 
    port* port;
    for(i = 0; i < SO_NAVI; i++){       //prima navi perchè altrimenti continuano a tentare l'accesso alla memoria condivisa del porto morto
        kill(shipList[i].pid, SIGINT);
    }    
    for(i = 0; i < SO_PORTI; i++){
        port = shmat(portList[i].keyPortMemory, NULL, 0); TEST_ERROR;
        kill(port->pid, SIGINT); TEST_ERROR;
        shmdt(port);
    }

}



void alarmHandler(int signum) {
    //printf("%d", signum); /*da cancellare*/
    if(signum == SIGINT)
        printf("DAY %d\n", *daysRemains);
    if (*daysRemains > 0) {
        // dump giornaliero 
        printf("[ REPORT PROVVISORIO ]\n\n"); 
      
        dumpSimulation(0); 
    } 
    *daysRemains-= 1; 
}

void dumpSimulation(int type) { 

    int i; 
    int j; 
    int k; 
    port* currentPort;
    good* currentOffer; 
    lot* currentLot;  
    int idGood; 
    int **totalGoods; 
    int **reportPorts; 
    int shipWithCargo = 0; 
    int shipWithoutCargo = 0; 
    int shipToPort = 0; 
    int semval; 
    /* final report variables */
    int **goodsReport; 
    int **maxOfferPort;
    int **maxRequestPort;  
    
    totalGoods= (int **)malloc(sizeof(int*)*(SO_MERCI + 1));
    for(i=0; i < SO_MERCI+1; i++){
        totalGoods[i] = (int *)malloc(sizeof(int)*5);
    }
    reportPorts = (int **)malloc(sizeof(int*)*SO_PORTI);
    for(i=0; i<SO_PORTI; i++){
        reportPorts[i] = (int *)malloc(sizeof(int)*6);
    }
    goodsReport = (int **)malloc(sizeof(int*)*(SO_MERCI + 1));
    for(i=0; i < SO_MERCI+1; i++){
        goodsReport[i] = (int *)malloc(sizeof(int)*5);
    }
    maxOfferPort = (int **)malloc(sizeof(int*)* (SO_MERCI + 1));
    for(i=0; i < SO_MERCI+1; i++){
        maxOfferPort[i] = (int *)malloc(sizeof(int)*2);
    }    
    maxRequestPort = (int **)malloc(sizeof(int*)* (SO_MERCI + 1));
    for(i=0; i < SO_MERCI+1; i++){
        maxRequestPort[i] = (int *)malloc(sizeof(int)*2);
    }


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
   

    initializeMatrix(reportPorts, SO_PORTI, 6);
    initializeMatrix(totalGoods, SO_MERCI+1, 5);
    if(type){
        initializeMatrix(goodsReport, SO_MERCI+1, 5);
    }   
    initializeMatrix(maxOfferPort, SO_MERCI+1, 2);
    initializeMatrix(maxRequestPort, SO_MERCI+1, 2);


    /* totalGoods ports and ships */

    for (i = 0; i < SO_PORTI; i++) {

        currentPort = shmat(portList[i].keyPortMemory, NULL, 1); 
        TEST_ERROR;
        reportPorts[i][0] = currentPort->pid; 
        reportPorts[i][3] = currentPort->inventory.request.amount -  currentPort->inventory.request.remains;
        
        semval = semctl(currentPort->sem_docks_id, 0, GETVAL); TEST_ERROR;
        reportPorts[i][4] = currentPort->totalDocks - semval; 
        reportPorts[i][5] = currentPort->totalDocks; 
        if (type) {
            if (currentPort->inventory.request.amount >= maxRequestPort[currentPort->inventory.request.idGood][0]) {
                maxRequestPort[currentPort->inventory.request.idGood][0] = currentPort->inventory.request.amount; 
                maxRequestPort[currentPort->inventory.request.idGood][1] = currentPort->pid; 
            }
        }
        currentOffer = shmat(currentPort->inventory.keyOffers, NULL, 1);TEST_ERROR;

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

            currentLot = shmat(currentOffer[j].keyLots, NULL, 1); TEST_ERROR;

            for (k = 0; k < currentOffer[j].maxLoots; k++) {


                if (currentLot->available) {
                    reportPorts[i][1] += currentLot->value; 
                } else {
                    reportPorts[i][2] += currentLot->value; 
                }

                idGood = currentLot[k].idGood; 
                switch (currentLot[k].status){
                    case 0:
                        if (currentLot[k].life > (SO_DAYS - (*daysRemains))) {
                            totalGoods[idGood][0] += currentLot[k].value; 
                        } else {
                            if (type) {
                                goodsReport[currentLot[k].idGood][2] += currentLot[k].value; 
                            }
                            totalGoods[idGood][3] += currentLot[k].value; 
                        }
                        break;
                    case 1: 
                        if (currentLot[k].life > (SO_DAYS - (*daysRemains))) {
                            printTest(totalGoods[idGood][1]);
                            printTest(currentLot[k].value);
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
            /*shmdt(currentLot); TEST_ERROR;*/
        } 
        /*shmdt(currentPort); TEST_ERROR; 
        shmdt(currentOffer); TEST_ERROR; */  
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

    /*printTotalGoods(totalGoods, SO_MERCI+1);
    printf("Il numero di navi con il carico a bordo è: %d\n", shipWithCargo); 
    printf("Il numero di navi senza carico a bordo è: %d\n", shipWithoutCargo); 
    printf("Il numero di navi presso un porto è: %d\n", shipToPort); */  
    printReportPorts(reportPorts, SO_PORTI);
   
    /*if final simulation => */
    /*if (type) {
        printGoodsReport(goodsReport, SO_MERCI+1);
        printMaxPort(maxOfferPort, maxRequestPort, SO_MERCI+1); 

    }*/
    printf("FINE REPORT\n");
    printTest(375);
}

void initializeMatrix(int **matrix, int r, int c) {
    int i; 
    int j; 
    for (i = 0; i < r; i++) {
        for (j = 0; j < c; j++) {
            matrix[i][j] = 0; 
        }
    }
}




/* INIZIO METODI PER LA STAMPA*/

void printTotalGoods (int **matrix, int lenght) {

    int i; 
    
    printf("ID merce | Presente in porto | Presente in nave | Consegnata ad un porto | Scaduta in porto | Scaduta in nave\n");

    // Stampa delle linee orizzontali prima dei dati
    printf("-------------------------------------------------------------------------------------------------------------\n");

    // Stampa della tabella con i dati
    for (i = 1; i < lenght; i++) {
        printf("%d | %20d | %20d | %20d | %20d | %20d\n",
           i, matrix[i][0], matrix[i][1], matrix[i][2], matrix[i][3], matrix[i][4]);

        // Stampa delle linee orizzontali tra le righe di dati
        printf("---------------------------------------------------------------------------------------------------------\n");
    }

}

void printReportPorts(int **matrix, int lenght) {

    int i; 

    printf("PID Porto | Presente Porto | Spedita dal Porto | Ricevuta in Porto | Banchine Occupate | Banchine Totali\n");

    // Stampa delle linee orizzontali prima dei dati
    printf("------------------------------------------------------------------------------------------------------------------------\n");

    // Stampa della tabella con i dati
    for (i = 0; i < lenght; i++) {
        printf("%9d | %15d | %14d | %17d | %18d | %17d\n",
            matrix[i][0], matrix[i][1], matrix[i][2], matrix[i][3], matrix[i][4], matrix[i][5]);

        // Stampa delle linee orizzontali tra le righe di dati
        printf("--------------------------------------------------------------------------------------------------------------------\n");
    }
}

void printGoodsReport(int **matrix, int lenght) {

    int i; 

    printf("ID merce | Quantità Generata | Rimasta Ferma | Scaduta in Porto | Scaduta in Nave | Consegnata\n");

    // Stampa delle linee orizzontali prima dei dati
    printf("-----------------------------------------------------------------------------------------------\n");

    // Stampa della tabella con i dati
    for (i = 1; i < lenght; i++) {
        printf("%4d | %16d | %13d | %16d | %15d | %10d\n",
            i,
            matrix[i][0],
            matrix[i][1],
            matrix[i][2],
            matrix[i][3],
            matrix[i][4]);

        // Stampa delle linee orizzontali tra le righe di dati
        printf("--------------------------------------------------------------------------------------------\n");
    }    
}

void printMaxPort(int **matrixOffer, int **matrixRequest, int lenght) {

    int i;  

    printf("ID merce | Porto con massima offerta | Quantità offerta | Porto massima richiesta | Quantità richiesta\n"); 
    
    // Stampa delle linee orizzontali prima dei dati
    printf("------------------------------------------------------------------------------------------------------\n");

    for (i = 1; i < lenght; i++) {
        printf("%20d | %20d | %20d | %20d | %20d\n",
            i, 
            matrixOffer[i][1], 
            matrixOffer[i][0], 
            matrixRequest[i][1], 
            matrixRequest[i][0]); 
        // Stampa delle linee orizzontali prima dei dati
        printf("------------------------------------------------------------------------------------------------------\n");
    }
}

/* FINE METODI PER LA STAMPA*/

int* getCasualWeight() {
    printTest(480);
    int *offer = (int*)malloc(SO_PORTI * sizeof(int));
    int sum = 0;
    int i;

    srand(time(NULL));
    for(i=0; i<SO_PORTI; i++){
        offer[i] = rand()%1000+1;
        sum += offer[i];
    }

    for(i=0; i<SO_PORTI; i++){
        offer[i] = (SO_FILL * offer[i])/ sum;
        if (offer[i] < 0) printTest(488);
        /*resto -= offer[i];*/
        if(offer[i] == 0) offer[i]++;
    }
    printTest(497);
    return offer; 
}



