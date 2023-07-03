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


ship* ship_list; //se puntatore freccia se è struttura .  
database* db;


int main(int argc,char **argv) {

    if(argc != 6){
        printf("error argc");
    }
    int handleProcess = 1;
    int semId = atoi(argv[5]); 
    ship_list = (ship*)shmat(atoi(argv[3]), NULL, 0); 
    int shipIndex = atoi(argv[1]);  
    db = (database*)shmat(atoi(argv[2]), NULL, 0); 
    initShip(&ship_list[shipIndex]); 

    struct sembuf sb; 
    bzero(&sb, sizeof(struct sembuf ));
    decreaseSem(sb, semId, 0);

    TEST_ERROR;

    waitForZero(sb, semId, 0);
    
    TEST_ERROR;

    // parte la simulazione 
    

    while (handleProcess) {

       handleProcess = findPorts(&ship_list[shipIndex]); 

    } 

    // ALTRIMENTI SLEEP 
    sleep(SO_DAYS); 

    // Scollega il segmento di memoria condiviso
    shmdt(db);
    exit(EXIT_SUCCESS); // Termina il processo figlio

}

void initShip(ship* ship_list) {
    struct timespec t; 
    // Inizializza i campi della struttura nave
    ship_list->pid = getpid();
    ship_list->keyOffer = -1;
    ship_list->keyRequest = -1; 
    
    // Genera coordinate casuali sulla mappa
    ship_list->position = (coordinate*)malloc(sizeof(coordinate));
    clock_gettime(CLOCK_REALTIME, &t);
    ship_list->position->x = (double)(t.tv_nsec%(SO_LATO*100))/ 100.0;
    clock_gettime(CLOCK_REALTIME, &t);
    ship_list->position->y = (double)(t.tv_nsec%(SO_LATO*100))/ 100.0;
 
   
}


double distance(coordinate* positionX, coordinate* positionY) {
    // ritorna la distanza tra porto e nave tramite la formula della distanza Euclidea
    double dx = positionY->x - positionX->x; 
    double dy = positionY->y - positionX->y; 

    return sqrt(dx * dx + dy * dy); 
}

void moveToPort(char type, lot* lots, int idGood, ship* ship) {
    
    double dist; 
    double travelTime; 
    
    dist = distance(ship_list->position, db[(type == 'o' ? ship_list->keyOffer : ship_list->keyRequest )].position); 
    travelTime = dist / SO_SPEED; 

    struct timespec sleepTime; 
    sleepTime.tv_sec = (int)travelTime; 
    sleepTime.tv_nsec = (double) travelTime - (double)(int)travelTime; 

    nanosleep(&sleepTime, NULL); 
    TEST_ERROR; 
    ship->position->x = db[ship_list->keyOffer].position->x; 
    ship->position->y = db[ship_list->keyOffer].position->y;  
    if (type == 'o') 
        loadLot(lots, idGood, ship);
    else 
        unloadLot(lots, ship); 

}


int findPorts(ship* ship_list) {

    port *portList;  
    good* offer; 
    lot* lots; 
    double distanzaMinima = -1;
    int nearestPort_index;
    int port[SO_PORTI]; // array dei porti offerta già controllati
    int j = 0; 
    int i; 
    int k; 
    int c;
    int findPorts = 0; 
    int idGood = -1; 
    lot* finalLot = NULL;
    struct sembuf sb; 
    bzero(&sb, sizeof(struct sembuf ));

    while (!findPorts || j != SO_PORTI) {
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
        portList =shmat(db[nearestPort_index].keyPortMemory, NULL, 0); TEST_ERROR; 
        offer = portList->inventory.offer;
       
        for(i = 0; i < portList->inventory.counterGoodsOffer && !findPorts; i++){
            lots = offer[i].lots; 

            for(c = 0; c < portList->inventory.offer[i].maxLoots && idGood == -1; c++) {
                decreaseSem(sb, portList->sem_inventory_id, 1);
                if ( lots[c].available != 0 && lots[c].id_ship != -1 ) {
                    idGood = offer[i].idGood; 
                    finalLot = &lots[c];
                }
                increaseSem(sb, portList->sem_inventory_id, 1);
            }
            if (idGood != -1) {
                if (findRequestPort(idGood, finalLot, ship_list)) {
                    findPorts = 1;  
                    break; 
                } else {
                    idGood = -1; 
                    offer++; // passa alla prossima offerta; 
                }
            }
        }

        if (!findPorts) {
            j++; 
        }
        shmdt(portList);
        TEST_ERROR;
    }

    if (findPorts) {
        ship_list->keyOffer = nearestPort_index; 
        finalLot->id_ship = ship_list->pid; 
        moveToPort('o', finalLot, idGood, ship_list); 
        moveToPort('r', finalLot, idGood, ship_list); 
    }


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
        if (portList->inventory.request.idGood == idGood && (portList->inventory.request.remains - portList->inventory.request.requestBooked ) > 0) {
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


void loadLot(lot* lots, int idGood, ship* ship_list) {

    struct sembuf sops; 
    port* port; 
    double quantita = lots->value;
    double tempoCaricamento = quantita / SO_SPEED;
    struct timespec sleepTime;
    sleepTime.tv_sec = (int)tempoCaricamento;
    sleepTime.tv_nsec = (tempoCaricamento - (int)tempoCaricamento) * 1e9;

    bzero(&sops, sizeof(struct sembuf)); // mette a zero tutti i valori di una struttura e serve per non rischiare di dare inf sbagliate alla semop

    port = shmat(db[ship_list->keyOffer].keyPortMemory, NULL, 0); 

    decreaseSem(sops, port->sem_docks_id, 0); 

    nanosleep(&sleepTime, NULL); TEST_ERROR;

    increaseSem(sops, port->sem_docks_id, 0); 

    decreaseSem(sops, port->sem_inventory_id, 1); // se offerta 1 se richiesta 0 
    lots->available = 0; 
    increaseSem(sops, port->sem_inventory_id, 1); 
    
    ship_list->listGoods.idGood = idGood; 
    
}

void unloadLot(lot* lots, ship* ship_list) {

    struct sembuf sops; 
    double x; 
    port* port; 
    double quantita = lots->value;
    double tempoCaricamento = quantita / SO_SPEED;
    struct timespec sleepTime;
    sleepTime.tv_sec = (int)tempoCaricamento;
    sleepTime.tv_nsec = (tempoCaricamento - (int)tempoCaricamento) * 1e9;

    bzero(&sops, sizeof(struct sembuf)); // mette a zero tutti i valori di una struttura e serve per non rischiare di dare inf sbagliate alla semop

    port = shmat(db[ship_list->keyRequest].keyPortMemory, NULL, 0); 

    decreaseSem(sops, port->sem_docks_id, 0); 

    nanosleep(&sleepTime, NULL); TEST_ERROR;

    increaseSem(sops, port->sem_docks_id, 0); 

    decreaseSem(sops, port->sem_inventory_id, 0); 

    x = lots->value > port->inventory.request.remains ? 0 : port->inventory.request.remains - lots->value; 
    port->inventory.request.remains = x; 
    port->inventory.request.requestBooked -= lots->value; 

    increaseSem(sops, port->sem_inventory_id, 0); 

}



