#include <stdlib.h>
#include <stdio.h>
#include <sys/sem.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>
#include <sys/shm.h>
#include <time.h>
#include "../macro/macro.h"
#include "../utility/utility.h"
#include "navi.h"

int numBytes;
char *string;
ship* ship_list;  
database* db;

int* dayRemains; 


int main(int argc, char **argv) {
    
    int handleProcess = 1;
    int semId = atoi(argv[5]); 
    int msg_id;
    int stop = 0;
    if(argc == 0) printf("errore\n");

   
    ship_list = shmat(atoi(argv[3]), NULL, 0); TEST_ERROR;
    dayRemains = shmat(atoi(argv[6]), NULL, 0); TEST_ERROR;
    //msg_id = shmat(atoi(argv[7]), NULL, 0); TEST_ERROR;

   
    int shipIndex = atoi(argv[1]);   
    db = shmat(atoi(argv[2]), NULL, 0); TEST_ERROR; 
    
    initShip(&ship_list[shipIndex]); 

    printf("fine init\n");
    struct sembuf sb; 
    bzero(&sb, sizeof(struct sembuf ));
    decreaseSem(sb, semId, 0);

    TEST_ERROR;

    waitForZero(sb, semId, 0);
    
    TEST_ERROR;

    /* parte la simulazione */
    while (handleProcess && *dayRemains > 0) {
        handleProcess = findPorts(&ship_list[shipIndex]);   
        if (!handleProcess) {
            stop = 1; 
            msgsnd(msgget(getppid(), 0600), &stop, sizeof(int), 0);
        }              
    } 

    pause(); 
    /* scollego segmento memoria condivisa */
    /*  shmdt(db); */
    exit(EXIT_SUCCESS); /* termina il processo figlio */
}

void initShip(ship* ship_list) {
    struct timespec t; 

    ship_list->pid = getpid();
    ship_list->keyOffer = -1;
    ship_list->keyRequest = -1; 
    ship_list->statusCargo = 0; 
    ship_list->statusPosition = 0; 
    
    clock_gettime(CLOCK_REALTIME, &t);
    ship_list->position.x = (double)(t.tv_nsec%(SO_LATO*100))/ 100.0;
    clock_gettime(CLOCK_REALTIME, &t);
    ship_list->position.y = (double)(t.tv_nsec%(SO_LATO*100))/ 100.0;
 
   
}


double distance(coordinate positionX, coordinate positionY) {

    /* formula distanza Euclidea */
    double dx = positionY.x - positionX.x; 
    double dy = positionY.y - positionX.y; 

    return sqrt(dx * dx + dy * dy); 
}

int moveToPort(char type, lot* lots, int idGood, ship* ship) {
    
    double dist; 
    double travelTime; 
    int done = 0;  

    struct timespec sleepTime; 

    dist = distance(ship_list->position, db[(type == 'o' ? ship_list->keyOffer : ship_list->keyRequest )].position); 
    travelTime = dist / SO_SPEED; 
     
    sleepTime.tv_sec = (int)travelTime; 
    sleepTime.tv_nsec = (travelTime - (int)sleepTime.tv_sec) * 1000000000; 

    
    nanosleep(&sleepTime, NULL); 
    TEST_ERROR; 
    
    if (type == 'o') { 
        done = loadLot(lots, idGood, ship);
    } else {
        unloadLot(lots, ship); 
    }
    return done; 

}


int findPorts(ship* ship_list) {

    int done; 
    port *portList;  
    good *offerList;  
    lot *lots; 
    double distanzaMinima = -1;
    int nearestPort_index;
    int port[SO_PORTI]; /* array per tenere traccia dei porti già controllati */
    int j = 0; 
    int i; 
    int k; 
    int c;
    int findPorts = 0; 
    int idGood = -1; 
    lot *finalLot;
    struct sembuf sb; 
    bzero(&sb, sizeof(struct sembuf ));

    
    while (!findPorts && j < SO_PORTI) {
        for (i = 0; i < SO_PORTI; i++) {
           
            int found = 0; 
            for (k = 0; k < j && !found; k++) { 
                if (i == port[k]) {
                    found = 1; 
                }
            }
         
            if (!found) {
               
                double distanza = distance(ship_list->position, db[i].position);
            
                if (distanzaMinima == -1 || distanza < distanzaMinima) {
                    distanzaMinima = distanza;
                    nearestPort_index = i;
                    port[j] = i; 
                }
            }
        
        } 
  
        portList = shmat(db[nearestPort_index].keyPortMemory, NULL, 0); TEST_ERROR; 
        offerList = shmat(portList->inventory.keyOffers, NULL, 0); TEST_ERROR; 

        for(i = 0; i < portList->inventory.counterGoodsOffer && !findPorts; i++) {         

            if (offerList[i].life >= (SO_DAYS - (*dayRemains))) {
                lots = shmat(offerList[i].keyLots, NULL, 0); 
                 for(c = 0; c < offerList[i].maxLoots && idGood == -1; c++) {
                    
                
                    
                    if (lots[c].available && lots[c].id_ship == -1 ) {
                        idGood = offerList[i].idGood; 
                        finalLot = &lots[c];   
                        //printf("OFFERTA IN NAVE : lotto %d-> id %d-- q %d\n", c, idGood, lots[c].value);
                    }
                
                }

            
                if (idGood != -1) {
                    if (findRequestPort(idGood, finalLot, ship_list)) {
                        findPorts = 1;  
                        
                        break; 
                    } else {
                        idGood = -1; 
                    }
                }
                shmdt(lots); 
            }
        }

        if (!findPorts) {
            j++; 
            distanzaMinima = -1;
        }
        shmdt(portList);      
        shmdt(offerList); 
    }
    if (findPorts) {
        ship_list->keyOffer = nearestPort_index; 
        finalLot->id_ship = ship_list->pid; 
        done = moveToPort('o', finalLot, idGood, ship_list); 
        if (done)  { /* se la merce è scaduta e quindi non è stata caricata non si muove verso la richiesta */
            moveToPort('r', finalLot, idGood, ship_list); 
        }
    } else {
        printTest(216); 
    }
   
    TEST_ERROR;

    return findPorts; 
}

int findRequestPort(int idGood, lot* lots, ship* ship_list) {
    int i; 
    port* portList; 
    int found; 
    struct sembuf sb;
    bzero(&sb, sizeof(struct sembuf)); //setta tutti i parametri a zero
    

    found = 0; 
    for (i = 0; i < SO_PORTI && !found; i++) {
        portList = shmat(db[i].keyPortMemory, NULL, 0); TEST_ERROR;
        decreaseSem(sb, portList->sem_inventory_id, 0);
        if (portList->inventory.request.idGood == idGood && (portList->inventory.request.remains - portList->inventory.request.requestBooked ) >= 0) {
            found = 1; 
        } 
        increaseSem(sb, portList->sem_inventory_id, 0);
        if (found) {
            decreaseSem(sb, portList->sem_inventory_id, 0);
            ship_list->keyRequest = i; 
            portList->inventory.request.requestBooked += lots->value; 
            increaseSem(sb, portList->sem_inventory_id, 0);
        }
        shmdt(portList); TEST_ERROR; 

    }

    return found;
} 


/* SPIEGAZIONE trovaPortoConRichiestaEquivalente: utilizziamo un ciclo while per confrontare ogni tipo di merce offerta da portoOfferta con la richiesta 
nel porto corrente. Se viene trovata una corrispondenza, blocciamo la richiesta nel porto trovato, prenotiamo il lotto corrispondente in portoOfferta 
e decrementiamo il semaforo della banchina. Se non viene trovata nessuna corrispondenza, passiamo all'iterazione successiva 
con il porto successivo fino a quando non viene trovato un porto con una richiesta equivalente. Se nessun porto con richiesta 
equivalente viene trovato, restituiamo NULL*/


int loadLot(lot* lots, int idGood, ship* ship_list) {

    int done = 0; 
    struct sembuf sops; 
    port* port; 
    int quantita = lots->value;
    int tempoCaricamento = quantita / SO_SPEED;
    struct timespec sleepTime;
    sleepTime.tv_sec = tempoCaricamento;
    sleepTime.tv_nsec = (tempoCaricamento - sleepTime.tv_sec) * 1e9;

    bzero(&sops, sizeof(struct sembuf)); /* mette a zero tutti i valori di una struttura e serve per non rischiare di dare inf sbagliate alla semop */

    port = shmat(db[ship_list->keyOffer].keyPortMemory, NULL, 0); 

    decreaseSem(sops, port->sem_inventory_id, 1); 
        if (lots->life > (SO_DAYS - (*dayRemains))) {
            ship_list->statusCargo = 1; 
            done = 1; 
            lots->available = 0; 
            lots->status = 1;
            ship_list->listGoods.idGood = idGood; 
        }    
    increaseSem(sops, port->sem_inventory_id, 1); 

    if (done) {

        ship_list->statusPosition = 1;

        decreaseSem(sops, port->sem_docks_id, 0); 

        nanosleep(&sleepTime, NULL); TEST_ERROR;

        increaseSem(sops, port->sem_docks_id, 0); 
        
        ship_list->statusPosition = 0; 
    }
    ship_list->position = db[ship_list->keyOffer].position;

    return done; 
}

void unloadLot(lot* lots, ship* ship_list) {

    struct sembuf sops; 
    int x; 
    port* port; 
    int quantita = lots->value;
    unsigned int tempoCaricamento = quantita / SO_SPEED;
    struct timespec sleepTime;
    sleepTime.tv_sec = tempoCaricamento;
    sleepTime.tv_nsec = (tempoCaricamento - sleepTime.tv_sec) * 1e9;

    ship_list->statusPosition = 1; 

    bzero(&sops, sizeof(struct sembuf)); // mette a zero tutti i valori di una struttura e serve per non rischiare di dare inf sbagliate alla semop

    port = shmat(db[ship_list->keyRequest].keyPortMemory, NULL, 0); 

    decreaseSem(sops, port->sem_docks_id, 0); 

    nanosleep(&sleepTime, NULL); TEST_ERROR;

    increaseSem(sops, port->sem_docks_id, 0); 

    decreaseSem(sops, port->sem_inventory_id, 0); 

    ship_list->statusPosition = 0; 
    ship_list->position = db[ship_list->keyRequest].position;

    /**
     * ? se la merce è scaduta la nave la butta 
    */
    
    if ((lots->life > (SO_DAYS - (*dayRemains)))) {
        x = lots->value > port->inventory.request.remains ? 0 : port->inventory.request.remains - lots->value; 
        lots->status = 2;
        port->inventory.request.remains = x; 
        port->inventory.request.requestBooked -= lots->value;      
    }

    ship_list->statusCargo = 0; 

    increaseSem(sops, port->sem_inventory_id, 0); 

}

/*int checkEconomy() {
    
    int i; 
    int j; 
    int k;
    int foundRequest = 0; 
    int res =0; 
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
}*/


