#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include "master.h";


port* portList;



int main(int argc, char **argv){
  int index = atoi(argv[1]);
  portList = (port*)shmat(atoi(argv[2]), NULL, 0); 
  initPort(index);
  
}




void initPort(int i) {
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

                    break;
            }
            portList[i].pid = pid; 
            

            int numRequest = rand() % SO_MERCI + 1; // numero casuale di tipi merci da assegnare alle richieste
            int numOffer = rand() % SO_MERCI + 1; // numero casuale di tipi merci da assegnare alle offerte
            while (numOffer + numRequest > 20) {
                numRequest = rand() % SO_MERCI + 1; // numero casuale di tipi merci da assegnare alle richieste
                numOffer = rand() % SO_MERCI + 1;
            }

            portList[i].inventory.request = (good*)malloc(numRequest * sizeof(good)); 
            portList[i].inventory.offer = (good*)malloc(numOffer * sizeof(good)); 

            initializeInventory(portList[i], numRequest, numOffer); 

            // assegno ai porti una quantità 

            
            //portList[i].inventory.request = (rand() % (SO_FILL + 1));
            portList[i].sem_docks_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666); // assegno semaforo 
            initSemaphore(portList[i]); // inizializzo il semaforo

            
        } 
    }
}
int isDuplicate(int numGood, int numRequest, int numOffer, char type) {
    int found = 0; 
    for (int x = 0; x < numRequest && !found; x++) {
        if (portList[i].inventory.request[x].idGood == numGood) {
            found = 1; 
        }
    }

    if (type == "offer") {
        for (int x = 0; x < numOffer && !found; x++) {
            if (portList[i].inventory.offer[x].idGood == numGood) {
                found = 1; 
            }
        }
    }

    return found; 
} 

void initializeInventory(port* portList, int numRequest, int numOffer) {
    for (int j = 0; j < numRequest; j++) {

        int numGood = rand() % SO_MERCI; 

        if (j != 0) { // controllo se la merce esiste già
            while (isDuplicate(numGood, numRequest, numOffer, "request")) {
                numGood = rand() % SO_MERCI; 
            }
        } 
        portList[i].inventory.request[j].idGood = numGood; 
        //int goodQuantity = rand() % SO_SIZE + 1; 
        int lifeTime = rand() % (SO_MAX_VITA + 1 - SO_MIN_VITA) + SO_MIN_VITA; 

    }

    for (int j = 0; j < numOffer; j++) {

        int numGood = rand() % SO_MERCI; 

        if (j != 0) { // controllo se la merce esiste già
            while (isDuplicate(numGood, numRequest, numOffer, "offer")) {
                numGood = rand() % SO_MERCI; 
            }
        } 
        portList[i].inventory.offer[j].idGood = numGood; 
        portList[i].inventory.offer[j].amount = 
        //int goodQuantity = rand() % SO_SIZE + 1; 
        int lifeTime = rand() % (SO_MAX_VITA + 1 - SO_MIN_VITA) + SO_MIN_VITA; 

    }
}

void initDocksSemaphore(port* portList) {

    if (portList.sem_docks_id == -1) {
        perror("semget"); 
        exit(EXIT_FAILURE); 
    }

    if (semctl(portList.sem_docks_id, 0, SETVAL, SO_BANCHINE) == -1) {
        perror("semctl"); 
        exit(EXIT_FAILURE); 
    }
}