#include "../macro/macro.h"

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