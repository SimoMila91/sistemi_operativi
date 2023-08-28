#ifndef _UTILITY_H
#define _UTILITY_H

void decreaseSem (struct sembuf sops, int sem_id, int sem_num); /* se offerta 1 se richiesta 0 */
void increaseSem (struct sembuf sops, int sem_id, int sem_num);
void waitForZero (struct sembuf sops, int sem_id, int sem_num);
void printTest(int riga);

#endif