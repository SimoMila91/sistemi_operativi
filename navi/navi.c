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

ship* shipList; //se puntatore freccia se è struttura . 
database* db;
int main(int argc, char **argv) {

    int handleSimulationState; 
    SO_SPEED = atoi(getenv("SO_SPEED")); 
    SO_LATO = atoi(getenv("SO_LATO")); 
    SO_PORTI = atoi(getenv("SO_PORTI")); 
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

    while (checkEconomy()) {

        findPorts(shipList[shipIndex]); 


    } 

    // ALTRIMENTI SLEEP 

    // ripeto tutto 
 
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

void moveToPort(char type, lot* lots) {
    
    double dist; 
    double travelTime; 
    if (type == 'o') {
    dist = distance(shipList->position, db[shipList->keyOffer].position); 
    travelTime = dist / SO_SPEED; 

    struct timespec sleepTime; 
    sleepTime.tv_sec = (int)travelTime; 
    sleepTime.tv_nsec = (double) travelTime - (double)(int)travelTime; 

    nanosleep(&sleepTime, NULL); 

    ship->position->x = db[shipList->keyOffer].position->x; 
    ship->position->y = db[shipList->keyOffer].position->y;  
    caricaLotto(lots);

    } else if (type == 'r') {

    }

    
  

    // carico o scarico 



}

int checkEconomy() {
    
    int i; 
    int j; 
    int k;
    int foundRequest = 0; 
    int res; 
    good* currentOffer; 
    port portList; 
    struct sembuf sb; 
    bzero(&sb, sizeof(struct sembuf ));
    
    for (i = 0; i < SO_PORTI && !res; i++) {
        portList = (port)shmat(db[i].keyPortMemory, NULL, 0); 
        decreaseSem(sb, portList.sem_inventory_id, 0 );
        if (portList.inventory.request.lots->available) {
            foundRequest = portList.inventory.request.idGood; 
        }   
        increaseSem(sb, portList.sem_inventory_id, 0);
        int found = 0;  
        for (j = 0; j < SO_PORTI && !found; j++) {
            currentOffer = (port)shmat(db[i].keyPortMemory, NULL, 1);  
            decreaseSem(sb, portList.sem_inventory_id, 0 );
            for(k = 0; k < portList.inventory.counterGoodsOffer &&  !found; k++ ){
                for(l = 0; l < portList.inventory.offer->maxLoots && !found; l++){
                    if (currentOffer[k].lots[l].available && currentOffer->idGood == foundRequest) {
                        found = 1; 
                    }   
                }
                
            }
            increaseSem(sb, portList.sem_inventory_id, 1);
            shmdt(currentOffer); TEST_ERROR; 
        }
        
        res = found;
        shmdt(portList); TEST_ERROR; 
    }
    if (shmdt(portList) == -1) {
        perror("shmdt: ship -> checkEconomy"); 
        exit(EXIT_FAILURE); 
    }

    return res;  
}

int findPorts(ship* shipList) {

    port portList;  // puntatore si o no? ainz 
    good* offer; 
    lot* lots; 
    double distanzaMinima = -1;
    int nearestPort_index;
    int port[SO_PORTI]; 
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

    while (!findPorts) {
        for (i = 0; i < SO_PORTI; i++) {
            int found = 0; 
            for (k = 0; k < j && !found; k++) { //////
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
                if (findRequestPort(idGood, finalLot)) {
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
    }


    return findPorts; 
}

int findRequestPort(int idGood, lot* lots) {
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


void caricaLotto(lot* lots ) {

    double quantita = lots->value;
    double tempoCaricamento = quantita / SO_SPEED;
    int i;
    good listGoods = shipList->listGoods;
    struct timespec sleepTime;
    sleepTime.tv_sec = (int)tempoCaricamento;
    sleepTime.tv_nsec = (tempoCaricamento - (int)tempoCaricamento) * 1e9;

    nanosleep(&sleepTime, NULL); TEST_ERROR;


        if (shipList->listGoods.idGood == lotto->idGood) {
            // Aggiorna la quantità della merce sulla nave
            nave->listGoods[i].amount += lotto->amount;
            break;
        }
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