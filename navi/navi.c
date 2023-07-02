#include <stdio.h>
#include <sys/sem.h>
#include "../master/master.h"
#include "navi.h"
#include "../macro/macro.h"
#include <sys/shm.h>
#include <time.h>

int SO_SPEED; 
double SO_LATO; 
int SO_PORTI;  
int SO_DAYS; 

ship* shipList; //se puntatore freccia se è struttura . 
database* db;
int main(int argc, char **argv) {

    int handleProcess = 1; 
    int handleSimulationState; 
    SO_SPEED = atoi(getenv("SO_SPEED")); 
    SO_LATO = atoi(getenv("SO_LATO")); 
    SO_PORTI = atoi(getenv("SO_PORTI")); 
    SO_DAYS = atoi(getenv("SO_DAYS")); 
    int semId = atoi(argv[5]); 
    shipList = (ship*)shmat(atoi(argv[3]), NULL, 0); 
    int shipIndex = atoi(argv[1]);  
    db = (database*)shmat(atoi(argv[2]), NULL, 0); 
    initShip(shipList[shipIndex]); 

    struct sembuf sb; 
    bzero(&sb, sizeof(struct sembuf ));
    decreaseSem(sb, semId, 0);

    TEST_ERROR;

    waitForZero(sb, semId, 0);
    
    TEST_ERROR;

    // parte la simulazione 
    

    while (handleProcess) {

       handleProcess = findPorts(shipList[shipIndex]); 

    } 

    // ALTRIMENTI SLEEP 
    sleep(SO_DAYS); 

    // Scollega il segmento di memoria condiviso
    shmdt(db);
    exit(EXIT_SUCCESS); // Termina il processo figlio

}

void initShip(ship* shipList) {
            
    // Inizializza i campi della struttura nave
    shipList->pid = getpid();
    shipList->keyOffer = -1;
    shipList->keyRequest = -1; 
    
    // Genera coordinate casuali sulla mappa
    shipList->position = (coordinate*)malloc(sizeof(coordinate));
    shipList->position->x = rand() % SO_LATO;
    shipList->position->y = rand() % SO_LATO;
    
    shipList->listGoods = NULL; 
   
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
    
    dist = distance(shipList->position, db[(type == 'o' ? shipList->keyOffer : shipList->keyRequest )].position); 
    travelTime = dist / SO_SPEED; 

    struct timespec sleepTime; 
    sleepTime.tv_sec = (int)travelTime; 
    sleepTime.tv_nsec = (double) travelTime - (double)(int)travelTime; 

    nanosleep(&sleepTime, NULL); 
    TEST_ERROR; 
    ship->position->x = db[shipList->keyOffer].position->x; 
    ship->position->y = db[shipList->keyOffer].position->y;  
    if (type == 'o') 
        loadLot(lots, idGood, ship);
    else 
        unloadLot(lots, idGood, ship); 

}


int findPorts(ship* shipList) {

    port portList;  // puntatore si o no? ainz 
    good* offer; 
    lot* lots; 
    double distanzaMinima = -1;
    int nearestPort_index;
    int port[SO_PORTI]; // array dei porti offerta già controllati
    int j = 0; 
    int i; 
    int k; 
    int c;
    int offerCounter; 
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
                double distanza = distance(shipList->position, db[i].position);

                if (distanzaMinima == -1 || distanza < distanzaMinima) {
                    distanzaMinima = distanza;
                    nearestPort_index = i;
                    port[j] = i; 
                }
            }
        
        }
        portList = (port)shmat(db[nearestPort_index].keyPortMemory, NULL, 0); TEST_ERROR; 
        offer = portList->inventory.offer;
       
        for(i = 0; i < portList.inventory.counterGoodsOffer && !findPorts; i++){
            lots = offer[i].lots; 

            for(c = 0; c < portList.inventory.offer[i].maxLoots && idGood == -1) {
                decreaseSem(sb, portList.sem_inventory_id, 1);
                if ( lots[c].available != 0 && lots[c].id_ship != -1 ) {
                    idGood = offer[i].idGood; 
                    finalLot = &lots[c];
                }
                increaseSem(sb, portList.sem_inventory_id, 1);
            }
            if (idGood != -1) {
                if (findRequestPort(idGood, finalLot, shipList)) {
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
        shipList->keyOffer = nearestPort_index; 
        finalLot->id_ship = shipList->pid; 
        moveToPort('o', finalLot, idGood, shipList); 
        moveToPort('r', finalLot, idGood, shipList); 
    }


    return findPorts; 
}

int findRequestPort(int idGood, lot* lots, ship* shipList) {
    int i; 
    port* portList; 
    int found; 
    

    found = 0; 
    for (i = 0; i < SO_PORTI && !found; i++) {
        portList = (port)shmat(db[i].keyPortMemory, NULL, 0); TEST_ERROR;
        decreaseSem(sb, portList.sem_inventory_id, 0);
        if (portList->inventory.request->idGood == idGood && (portList->inventory.request->remains - portList->inventory.request->requestBooked ) > 0) {
            found = 1; 
        } 
        increaseSem(sb, portList.sem_inventory_id, 0);
        if (found) {
            decreaseSem(sb, portList.sem_inventory_id, 0);
            shipList->keyRequest = i; 
            portList->inventory.request->requestBooked += lots->value; 
            increaseSem(sb, portList.sem_inventory_id, 0);
        }
        shmdt(portList); TEST_ERROR; 
    }
} 


/* SPIEGAZIONE trovaPortoConRichiestaEquivalente: utilizziamo un ciclo while per confrontare ogni tipo di merce offerta da portoOfferta con la richiesta 
nel porto corrente. Se viene trovata una corrispondenza, blocciamo la richiesta nel porto trovato, prenotiamo il lotto corrispondente in portoOfferta 
e decrementiamo il semaforo della banchina. Se non viene trovata nessuna corrispondenza, passiamo all'iterazione successiva 
con il porto successivo fino a quando non viene trovato un porto con una richiesta equivalente. Se nessun porto con richiesta 
equivalente viene trovato, restituiamo NULL*/


void loadLot(lot* lots, int idGood, ship* shipList) {

    struct sembuf sops; 
    port* port; 
    double quantita = lots->value;
    double tempoCaricamento = quantita / SO_SPEED;
    int i;
    good listGoods = shipList->listGoods;
    struct timespec sleepTime;
    sleepTime.tv_sec = (int)tempoCaricamento;
    sleepTime.tv_nsec = (tempoCaricamento - (int)tempoCaricamento) * 1e9;

    bzero(&sops, sizeof(sembuf)); // mette a zero tutti i valori di una struttura e serve per non rischiare di dare inf sbagliate alla semop

    port = shmat(db[shipList->keyOffer].keyPortMemory, NULL, 0); 

    decreaseSem(sops, port->sem_docks_id, 0); 

    nanosleep(&sleepTime, NULL); TEST_ERROR;

    increaseSem(sops, port->sem_docks_id, 0); 

    decreaseSem(sops, port->sem_inventory_id, 1); // se offerta 1 se richiesta 0 
    lots->available = 0; 
    increaseSem(sops, port->sem_inventory_id, 1); 
    
    shipList->listGoods.idGood = idGood; 
    
}

void unloadLot(lot* lots, int idGood, ship* shipList) {

    struct sembuf sops; 
    double x; 
    port* port; 
    double quantita = lots->value;
    double tempoCaricamento = quantita / SO_SPEED;
    int i;
    good listGoods = shipList->listGoods;
    struct timespec sleepTime;
    sleepTime.tv_sec = (int)tempoCaricamento;
    sleepTime.tv_nsec = (tempoCaricamento - (int)tempoCaricamento) * 1e9;

    bzero(&sops, sizeof(sembuf)); // mette a zero tutti i valori di una struttura e serve per non rischiare di dare inf sbagliate alla semop

    port = shmat(db[shipList->keyRequest].keyPortMemory, NULL, 0); 

    decreaseSem(sops, port->sem_docks_id, 0); 

    nanosleep(&sleepTime, NULL); TEST_ERROR;

    increaseSem(sops, port->sem_docks_id, 0); 

    decreaseSem(sops, port->sem_inventory_id, 0); 

    x = lots->value > port->inventory.request.remains ? 0 : port->inventory.request.remains - lots->value; 
    port->inventory.request.remains = x; 
    port->inventory.request.requestBooked -= lots->value; 

    increaseSem(sops, port->sem_inventory_id, 0); 

}



void decreaseSem (struct sembuf sops, int sem_id, int sem_num){
    sops.sem_num = sem_num;
    sops.sem_op = -1; 
    sops.sem_flg = 0; 

    semop(sem_id, sops, 1); 
    TEST_ERROR;
}

void increaseSem (struct sembuf sops, int sem_id, int sem_num){
    sops.sem_num = sem_num;
    sops.sem_op = 1; 
    sops.sem_flg = 0; 

    semop(sem_id, sops, 1); 
    TEST_ERROR;
}


void waitForZero (struct sembuf sops, int sem_id, int sem_num){
    sops.sem_num = sem_num;
    sops.sem_op = 0; 
    sops.sem_flg = 0; 

    semop(sem_id, sops, 1); 
    TEST_ERROR;
}