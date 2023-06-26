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

ship* shipList; 

int main(int argc, char **argv) {

    int handleSimulationState; 
    SO_SPEED = atoi(getenv("SO_SPEED")); 
    SO_LATO = atoi(getenv("SO_LATO")); 
    SO_PORTI = atoi(getenv("SO_PORTI")); 
    int semId = atoi(argv[5]); 
    shipList = (ship*)shmat(atoi(argv[3]), NULL, 0); 
    int shipIndex = atoi(argv[1]);  
    initShip(shipList[shipIndex]); 

    struct sembuf sb; 
    sb.sem_num = 0; 
    sb.sem_op = -1; 
    sb.sem_flg = 0; 

    if (semop(semId, &sb, 1) == -1) {
        perror("semop ship"); 
        exit("EXIT FAILURE"); 
    }

    sb.sem_num = 0; 
    sb.sem_op = 0; 
    sb.sem_flg = 0; 
    
    if (semop(semId, &sb, 1) == -1) {
        perror("semop ship"); 
        exit("EXIT FAILURE"); 
    }

    // parte la simulazione 

    while (checkEconomy()) {
        
        // cerco la richiesta del porto più vicino // se non ci sono più offerte o non ci sono più richieste sleep 


        // cerco il porto più vicino che soddisfi la richiesta 
        // mi muovo verso il porto e carico 
        // mi muovo verso la richiesta e scarico 

    } 

    // ALTRIMENTI SLEEP 

  

    // ripeto tutto 
 



    
    // Scollega il segmento di memoria condiviso

    exit(EXIT_SUCCESS); // Termina il processo figlio

}

void initShip(ship* shipList) {
            
    // Inizializza i campi della struttura nave
    shipList->pid = getpid();
    
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

void moveToPort(ship* ship, port* destination, char type) {
    
    double dist; 
    double travelTime; 

    dist = distance(ship->position, destination->position); 
    travelTime = dist / SO_SPEED; 

    struct timespec sleepTime; 
    sleepTime.tv_sec = (int)travelTime; 
    sleepTime.tv_nsec = (double) travelTime - (double)(int)travelTime; 

    nanosleep(&sleepTime, NULL); 

    ship->position->x = destination->position->x; 
    ship->position->y = destination->position->y; 

    // carico o scarico 

    // aggiungere sincronizzazione

}

int checkEconomy() {
    
    int i; 
    int j; 
    int foundRequest = 0; 
    int res; 
    good* currentOffer; 
    port* portList = (port*)shmat(atoi(argv[2]), NULL, 0); 
    
    for (i = 0; i < SO_PORTI && !res; i++) {
        if (portList[i].inventory.request[0].lots->available) {
            foundRequest = portList[i].inventory.request->idGood; 
        } 
        int found = 0; 
        for (j = 0; j < SO_PORTI && !found; j++) {
            currentOffer = portList[i].inventory.offer; 

            while (currentOffer != NULL && !found) {
                if (currentOffer->lots->available && currentOffer->idGood == foundRequest) {
                    found = 1; 
                }
            }

        }
        
        res = found;

    }

    return res;  
}