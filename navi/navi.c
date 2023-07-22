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


ship* ship_list;  
database* db;
time_t startTime, currentTime; 
int dayRemains; 


int main(int argc, char **argv) {
    
    int shipMemoryKey; 
    int handleProcess = 1;
    int semId = atoi(argv[5]); 

    shipMemoryKey = atoi(argv[3]); 
   
    ship_list = shmat(atoi(argv[3]), NULL, 0); TEST_ERROR;

    dayRemains = SO_DAYS; 
    time_t startTime, currentTime; 
   
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
    

    while (handleProcess && dayRemains > 0) {

        currentTime = time(NULL); 
        if (currentTime - startTime >= 1) {
            dayRemains--; 
            startTime = currentTime; 
        }
        handleProcess = findPorts(&ship_list[shipIndex]); 

    } 

    pause(); 

    printf("ci siamo dio can");
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
    
    dist = distance(ship_list->position, db[(type == 'o' ? ship_list->keyOffer : ship_list->keyRequest )].position); 
    travelTime = dist / SO_SPEED; 

    struct timespec sleepTime; 
    sleepTime.tv_sec = (int)travelTime; 
    sleepTime.tv_nsec = (double) travelTime - (double)(int)travelTime; 

    nanosleep(&sleepTime, NULL); 
    TEST_ERROR; 
    ship->position.x = db[ship_list->keyOffer].position.x; 
    ship->position.y = db[ship_list->keyOffer].position.y;  
    if (type == 'o') 
       done = loadLot(lots, idGood, ship);
    else 
        unloadLot(lots, ship); 

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

    
    while (!findPorts || j < SO_PORTI) {
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


            if (offerList[i].life < (SO_DAYS - dayRemains)) {
                
                lots = shmat(offerList[i].keyLots, NULL, 0); 

                 for(c = 0; c < offerList[i].maxLoots && idGood == -1; c++) {
                
               
                    if (lots[c].available && lots[c].id_ship == -1 ) {
                        idGood = offerList[i].idGood; 
                        finalLot = &lots[c];    
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
        }
      
    }

    if (findPorts) {
        ship_list->keyOffer = nearestPort_index; 
        finalLot->id_ship = ship_list->pid; 
        done = moveToPort('o', finalLot, idGood, ship_list); 
        if (done)  { /* se la merce è scaduta e quindi non è stata caricata non si muove verso la richiesta */
             moveToPort('r', finalLot, idGood, ship_list); 
        }
    } else {
        printf("PORTI NON TROVATI\n"); 
    }

    shmdt(portList);
    shmdt(offerList); 
   
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


int loadLot(lot* lots, int idGood, ship* ship_list) {

    int done = 0; 
    struct sembuf sops; 
    port* port; 
    int quantita = lots->value;
    int tempoCaricamento = quantita / SO_SPEED;
    struct timespec sleepTime;
    sleepTime.tv_sec = tempoCaricamento;
    sleepTime.tv_nsec = (tempoCaricamento - tempoCaricamento) * 1e9;

    bzero(&sops, sizeof(struct sembuf)); /* mette a zero tutti i valori di una struttura e serve per non rischiare di dare inf sbagliate alla semop */

    port = shmat(db[ship_list->keyOffer].keyPortMemory, NULL, 0); 
 
    decreaseSem(sops, port->sem_inventory_id, 1); 
        if (lots->life < (SO_DAYS - dayRemains)) {
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

    return done; 
}

void unloadLot(lot* lots, ship* ship_list) {

    struct sembuf sops; 
    int x; 
    port* port; 
    int quantita = lots->value;
    int tempoCaricamento = quantita / SO_SPEED;
    struct timespec sleepTime;
    sleepTime.tv_sec = tempoCaricamento;
    sleepTime.tv_nsec = (tempoCaricamento - tempoCaricamento) * 1e9;

    ship_list->statusPosition = 1; 

    bzero(&sops, sizeof(struct sembuf)); // mette a zero tutti i valori di una struttura e serve per non rischiare di dare inf sbagliate alla semop

    port = shmat(db[ship_list->keyRequest].keyPortMemory, NULL, 0); 

    decreaseSem(sops, port->sem_docks_id, 0); 

    nanosleep(&sleepTime, NULL); TEST_ERROR;

    increaseSem(sops, port->sem_docks_id, 0); 

    decreaseSem(sops, port->sem_inventory_id, 0); 

    ship_list->statusPosition = 0; 

    /**
     * ? se la merce è scaduta la nave la non scarica e la butta 
    */
    if (!(lots->life > (SO_DAYS - dayRemains))) {
        x = lots->value > port->inventory.request.remains ? 0 : port->inventory.request.remains - lots->value; 
        lots->status = 2;
        port->inventory.request.remains = x; 
        port->inventory.request.requestBooked -= lots->value;      
    }

    ship_list->statusCargo = 0; 

    increaseSem(sops, port->sem_inventory_id, 0); 

}



