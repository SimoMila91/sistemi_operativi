#include <stdlib.h>
#include <stdio.h>
#include "master.h"; 

int SO_PORTI; 
int SO_NAVI; 
double SO_LATO;  

int shmid_port[SO_PORTI];
port* portList[SO_PORTI]; 

int main(int argc, char **argv) {

    init_var();  // initialize of variables 
    coordinate* map = (coordinate*) malloc(SO_LATO * SO_LATO * sizeof(coordinate)); 
 
    initPort(portList, )
    createSimulation(shmid_port, portList); // init of ports, ships and goods

}


void init_var() {
    if (getenv("SO_PORTI")) {
        SO_PORTI = atoi(getenv("SO_PORTI")); 
        SO_NAVI = atoi(getenv("SO_NAVi")); 
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
        
        pid_t pid = fork(); 
        if (pid < 0) {
            printf("Error in fork of port's process\n"); 
            exit(EXIT_FAILURE); 
        } else if (pid == 0) { // processo figlio 

            port *porto = (port*) shmat(shmid_port, NULL, 0); 


        } else { // processo padre

        }


    }
}