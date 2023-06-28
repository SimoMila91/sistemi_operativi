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
        
        trovaPortoPiuVicinoConOfferta();
        trovaPortoConRichiestaEquivalente();

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

// da rivedere 


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



port* trovaPortoPiuVicinoConOfferta(ship* nave, port* porti) {

    database* db = (database*)shmat(atoi(argv[2]), NULL, 0); 
    port portList; 
    double distanzaMinima = -1;
    port* portoPiuVicino = NULL;

    for (int i = 0; i < SO_PORTI; i++) {
        portList = (port)shmat(db[i]->keyPortMemory, NULL, 0);

        if (portList.inventory.offer != NULL && portList.inventory.offer->amount > 0) {
            double distanza = distance(nave->position, portList.position);

            if (distanzaMinima == -1 || distanza < distanzaMinima) {
                distanzaMinima = distanza;
                portoPiuVicino = portList;
            }
        }
    }

    return portoPiuVicino;
}

port* trovaPortoConRichiestaEquivalente(port* porti, port* portoOfferta, ship* ship) {

    database* db = (database*)shmat(atoi(argv[2]), NULL, 0); 
    port portList; 
    
    for (int i = 0; i < SO_PORTI; i++) {
        portList = (port)shmat(db[i]->keyPortMemory, NULL, 0);

            if (portList != portoOfferta && portList.inventory.request != NULL) {
                // Verifica se il porto richiede uno dei tipi di merce offerti da portoOfferta
                good* richiesta = portList.inventory.request;
                good* offerta = portoOfferta->inventory.offer;

                while (offerta != NULL) {
                    if (richiesta->idGood == offerta->idGood) {
                        // Blocca la richiesta nel porto trovato
                        richiesta->lots->id_ship = ship->pid;

                        // Prenota il lotto in portoOfferta
                        offerta->lots->id_ship = ship->pid ;

                        return &portList;
                    }

                    offerta = offerta->next;
                }
            }
        }

    return NULL; // Nessun porto con richiesta equivalente trovato
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

