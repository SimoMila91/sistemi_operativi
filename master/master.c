#include <stdlib.h>
#include <stdio.h>
#include "master.h"; 

int SO_PORTI; 
int SO_NAVI; 
double SO_LATO;  

int shmid_port[SO_PORTI];
port* portList[SO_PORTI]; 
int shmid_nave[SO_NAVI];
nave* naveList[SO_NAVI];

int main(int argc, char **argv) {

    init_var();  // initialize of variables 
    coordinate* map = (coordinate*) malloc(SO_LATO * SO_LATO * sizeof(coordinate)); 
 
    initPort(portList, )
    createSimulation(shmid_port, portList); // init of ports, ships and goods

}


void init_var() {
    if (getenv("SO_PORTI")) {
        SO_PORTI = atoi(getenv("SO_PORTI")); 
        SO_NAVI = atoi(getenv("SO_NAVI")); 
        SO_LATO = atoi(getenv("SO_LATO")); 
    } else {
        printf("Environment variables are not initialized");
        exit(EXIT_FAILURE); 
    }
}

void initPort(port *portList, int shmid_port)
{
    int i; 
    for (i = 0; i < SO_PORTI; i++) {
        //portList[i].pid = fork();
        pid_t pid = fork(); 
        if (pid < 0) {
            printf("Error in fork of port's process\n"); 
            exit(EXIT_FAILURE); 
       /* } else if (pid == 0) { // processo figlio 

            port *porto = (port*) shmat(shmid_port, NULL, 0); */


        } else { // processo padre e figlio 
            port *porto = (port*) shmat(shmid_port, NULL, 0); 
        }


    }
}


void initPort(nave *naveList, int shmid_nave)
{
    int i; 
    for (i = 0; i < SO_NAVI; i++) {
        //naveList[i].pid = fork();
        pid_t pid = fork(); 
        if (pid < 0) {
            printf("Error in fork of port's process\n"); 
            exit(EXIT_FAILURE); 
       /* } else if (pid == 0) { // processo figlio 

            port *porto = (port*) shmat(shmid_port, NULL, 0); */


        } else { // processo padre e figlio 
            nave *nave1 = (nave*) shmat(shmid_nave, NULL, 0); 
        }


    }
}



void creoPorti(port *portList, int shmid_port){
    int i;
    for(i = 0; i< SO_PORTI; i++){
        switch (i)
        {
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
            portList[2].position.y = SO_LATO;
            break;

        case 3: 
            portList[3].position->x = SO_LATO;
            portList[3].position->y = 0; 
            break;
        
        default:
            portList[i].position->x = (rand()%(SO_LATO + 1));

            break;
        }
    }

    portList[i].freeDocks = (rand()%(SO_BANCHINE)+1);
    portList[i].inventory.offer = (rand()%(SO_FILL + 1));
    portList[i].inventory.request = (rand()%(SO_FILL + 1));
}


void creoNavi(nave* naveList; int shmid_nave){
    int i;
    for(i = 0; i< SO_NAVI; i++){
        naveList[i].position->x = (rand()%(SO_LATO + 1));
        naveList[i].position->y = (rand()%(SO_LATO + 1));
        naveList[i].totalCargo = SO_CAPACITY;
    }

}




