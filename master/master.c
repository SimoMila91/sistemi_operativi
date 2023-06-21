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

int SO_PORTI; 
int SO_NAVI;  
double SO_LATO;  
int SO_BANCHINE; 
int SO_MERCI; 
int SO_SIZE; 
int SO_MAX_VITA; 
int SO_MIN_VITA; 
int SO_FILL; 

// port's variables 
int shmid_port; 
port* portList;

// ship's variables 
int shmid_nave;
ship* shipList;



int main(int argc, char **argv) {

    init_var();  // initialize of variables 
    coordinate* map = (coordinate*) malloc(SO_LATO * SO_LATO * sizeof(coordinate)); 
    

    shmid_port = createSharedMemory(sizeof(port) * SO_PORTI); 
    portList = (port*)shmat(shmid_port, NULL, 0); 
    initPort(portList);
    
    initShip(shipList, shmid_nave);
}

int createSharedMemory(size_t size) {
    key_t key = ftok(".", 'S'); // genera la chiave per la memoria condivisa
    int shmid = shmget(key, size, IPC_CREATE | 0666); // crea la memoria condivisa

    if (shmid == -1) {
        perror("shmget"); 
        exit(EXIT_FAILURE); 
    }
    
    return shmid; 
}

void init_var() {
    if (getenv("SO_PORTI")) {
        SO_PORTI = atoi(getenv("SO_PORTI")); 
        SO_NAVI = atoi(getenv("SO_NAVI")); 
        SO_LATO = atoi(getenv("SO_LATO")); 
        SO_BANCHINE = atoi(getenv("SO_BANCHINE")); 
        SO_MERCI = atoi(getenv("SO_MERCI")); 
        SO_SIZE = atoi(getenv("SO_SIZE")); 
        SO_MAX_VITA = atoi(getenv("SO_MAX_VITA")); 
        SO_MIN_VITA = atoi(getenv("SO_MIN_VITA")); 
        SO_FILL = atoi(getenv("SO_FILL")); 
    } else {
        printf("Environment variables are not initialized");
        exit(EXIT_FAILURE); 
    }
}


void initPort(port *portList) {

    int i;
    for (i = 0; i< SO_PORTI; i++) {
       pid_t pid = fork();
       if (pid < 0) {
        printf("Error in fork of port's process\n"); 
        exit(EXIT_FAILURE); 
       } else if (pid == 0) {
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

void initSemaphore(port* portList) {

    if (portList.sem_docks_id == -1) {
        perror("semget"); 
        exit(EXIT_FAILURE); 
    }

    if (semctl(portList.sem_docks_id, 0, SETVAL, SO_BANCHINE) == -1) {
        perror("semctl"); 
        exit(EXIT_FAILURE); 
    }
}

void initShip(ship *naveList, int shmid_nave)
{
    int i;
    for (i = 0; i < SO_NAVI; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            printf("Error in fork of ship's process\n");
            exit(EXIT_FAILURE);
        } else if (pid == 0) { // child process
            ship *currentShip = (ship*)shmat(shmid_nave[i], NULL, 0);
            
            // Inizializza i campi della struttura nave
            currentShip->pid = getpid();
            
            // Genera coordinate casuali sulla mappa
            currentShip->position = (coordinate*)malloc(sizeof(coordinate));
            currentShip->position->x = rand() % SO_LATO;
            currentShip->position->y = rand() % SO_LATO;
            
            currentShip->listGoods = NULL; // Assegna un valo re appropriato
            currentShip->totalCargo = 0.0; // Assegna un valore appropriato
            
            // Esegui la logica specifica della nave

            
            // Scollega il segmento di memoria condiviso
            exit(EXIT_SUCCESS); // Termina il processo figlio
        }
    }
}

double distance(coordinate* positionX, coordinate* positionY) {
    // ritorna la distanza tra porto e nave tramite la formula della distanza Euclidea
    double dx = positionY->x - positionX->x; 
    double dy = positionY->y - positionX->y; 

    return sqrt(dx * dx + dy * dy); 
}
