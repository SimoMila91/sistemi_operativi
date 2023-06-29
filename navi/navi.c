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

        findPorts(shipList[shipIndex]); 

        // Verifica se c'è una banchina libera
        struct sembuf sb;
        sb.sem_num = sem_docks_id;  // Indice del semaforo delle banchine
        sb.sem_op = 0;             // Controllo se il valore del semaforo è 0
        sb.sem_flg = IPC_NOWAIT;   // Non bloccare il processo

        int semStatus = semop(sem_id, &sb, 1);
        if (semStatus == -1) {
            if (errno == EAGAIN) {
                // Non ci sono banchine libere, la nave deve attendere
                sb.sem_op = -1;  // Decremento del semaforo di 1 (blocco la nave)
                sb.sem_flg = 0;
                semop(sem_id, &sb, 1);
                // La nave attende finché non c'è una banchina libera
            } else {
                // Errore nell'accesso al semaforo delle banchine
                perror("Errore nell'accesso al semaforo delle banchine");
                // Gestisci il caso appropriato
            }
        } else {
            // C'è una banchina libera, prenotala
            sb.sem_op = -1;  // Decremento del semaforo di 1 (prenotazione della banchina)
            sb.sem_flg = 0;
            semop(sem_id, &sb, 1);

            // Sposta la nave verso il portoOfferta
            moveToPort(shipList[shipIndex], portoOfferta, 'O');
            // Dopo il movimento, puoi eseguire altre azioni necessarie o procedere con lo scarico o il caricamento
        }

                // Carica il lotto prenotato
        caricaLotto(&shipList[shipIndex], portoOfferta->inventory.offer[dockIndex].lots);

        // Attendi finché non c'è una banchina libera nel porto della richiesta
        while (1) {
            int semStatus = semop(sem_docks_id_richiesta, &sb_richiesta, 1);
            if (semStatus == -1) {
                if (errno == EAGAIN) {
                    // Non ci sono banchine libere nel porto della richiesta, la nave deve attendere
                    continue;
                } else {
                    // Errore nell'accesso al semaforo delle banchine del porto della richiesta
                    perror("Errore nell'accesso al semaforo delle banchine del porto della richiesta");
                }
            } else {
                // C'è una banchina libera nel porto della richiesta, prenotala
                sb_richiesta.sem_op = -1;  // Decremento del semaforo di 1 (prenotazione della banchina nel porto della richiesta)
                sb_richiesta.sem_flg = 0;
                semop(sem_docks_id_richiesta, &sb_richiesta, 1);
                break;
            }
        }

        // Sposta la nave verso il porto della richiesta
        moveToPort(&shipList[shipIndex], portoRichiesta, 'R');

        // Rilascia la banchina nel portoOfferta
        sb.sem_op = 1;  // Incremento del semaforo di 1 (rilascio della banchina in portoOfferta)
        semop(sem_docks_id, &sb, 1);




        moveToPort();
        ------> libero semaforo 
        moveToPort();
        scaricalotto(); /////////// --- > 

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

void moveToPort(ship* ship, char type) {
    
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
    
    if (type == 'o') {
        
    } else if (type == 'r') {

    }

    // carico o scarico 

    // aggiungere sincronizzazione

}

int checkEconomy() {
    
    int i; 
    int j; 
    int foundRequest = 0; 
    int res; 
    good* currentOffer; 
    database* db = (database*)shmat(atoi(argv[2]), NULL, 0); 
    port portList; 

    
    for (i = 0; i < SO_PORTI && !res; i++) {
        portList = (port)shmat(db->keyPortMemory, NULL, 0); 

        if (portList.inventory.request[0].lots->available) {
            foundRequest = portList.inventory.request[0]->idGood; ///////????????
        } 
        int found = 0; 
        for (j = 0; j < SO_PORTI && !found; j++) {
            currentOffer = portList.inventory.offer; 

            while (currentOffer != NULL && !found) {
                if (currentOffer->lots->available && currentOffer->idGood == foundRequest) {
                    found = 1; 
                }
            }

        }
        
        res = found;

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
    int nearestPortKey = NULL;
    int port[SO_PORTI]; 
    int j = 0; 
    int i; 
    int k; 
    int offerCounter; 
    int findPorts = 0; 
    database* db = (database*)shmat(atoi(argv[2]), NULL, 0); 

    while (!findPorts) {
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
                    nearestPortKey = db[i].keyPortMemory;
                    port[j] = i; 
                }
            }
        
        }
        portList = (port)shmat(nearestPortKey, NULL, 0);
        offer = portList->inventory.offer;
        int idGood = -1; 
        
        while (offer != NULL && !findPorts) {
            lots = offer->lots; 
            while(lots != NULL && idGood == -1) {
                if (lots->id_ship != -1 && lots->available != 0) {
                    idGood = offer->idGood; 
                } else {
                    lots++; 
                }
            }
            if (idGood != -1) {
                if (findRequestPort(idGood, db, shipList)) {
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
    }

    if (findPorts) {
        shipList->keyOffer = nearestPortKey; 
        lots->id_ship = shipList->pid; 
    }

    if (shmdt(portList) == -1) {
        perror("shmdt: portList -> findPorts"); 
        exit(EXIT_FAILURE); 
    }

    if (shmdt(db) == -1) {
        perror("shmdt: db -> findPorts"); 
        exit(EXIT_FAILURE); 
    }

    return findPorts; 
}

int findRequestPort(int idGood,  database* db, ship* shipList) {
    int i; 
    port* portList; 
    int found; 

    found = 0; 
    for (i = 0; i < SO_PORTI && !found; i++) {
        portList = (port)shmat(db[i].keyPortMemory, NULL, 0);
        if (portList->inventory.request->idGood == idGood && portList->inventory.request->remains != 0 &&  portList->inventory.request->requestBooked == 0) {
            found = 1; 
        } 
        if (found) {
            shipList->keyRequest = db[i].keyPortMemory; 
            portList->inventory.request->requestBooked = 1; 
            if (shmdt(portList) == -1) {
                perror("shmdt: ship -> findRequestPort"); 
                exit(EXIT_FAILURE); 
            }
        }
    }
} 


/* SPIEGAZIONE trovaPortoConRichiestaEquivalente: utilizziamo un ciclo while per confrontare ogni tipo di merce offerta da portoOfferta con la richiesta 
nel porto corrente. Se viene trovata una corrispondenza, blocciamo la richiesta nel porto trovato, prenotiamo il lotto corrispondente in portoOfferta 
e decrementiamo il semaforo della banchina. Se non viene trovata nessuna corrispondenza, passiamo all'iterazione successiva 
con il porto successivo fino a quando non viene trovato un porto con una richiesta equivalente. Se nessun porto con richiesta 
equivalente viene trovato, restituiamo NULL*/


void caricaLotto(ship* ship, port* portoOfferta ) {

    double quantita = lotto->amount;
    double tempoCaricamento = quantita / SO_SPEED;
    int i;
    good* listGoods = ship->listGoods;
    lot* 
    struct timespec sleepTime;
    sleepTime.tv_sec = (int)tempoCaricamento;
    sleepTime.tv_nsec = (tempoCaricamento - (int)tempoCaricamento) * 1e9;

    nanosleep(&sleepTime, NULL);

    while()
    for (i = 0; i < MAX_GOODS; i++) {
        if (ship->listGoods[i].idGood == lotto->idGood) {
            // Aggiorna la quantità della merce sulla nave
            nave->listGoods[i].amount += lotto->amount;
            break;
        }
    }
}

