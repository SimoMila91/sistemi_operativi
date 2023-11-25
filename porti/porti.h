#ifndef _PORTI_H
#define _PORTI_H


void initPort(int i, int totalOffer, int totalRequest);

void remove_ipcs();

double distance(coordinate positionX, coordinate positionY);

int isDuplicate(int numGood, int numOffer);

void initializeInventory(int totalOffer, int totalRequest);

int initDocksSemaphore();

void createLoots(int amount, int index, int lifetime, int idGood);

int* getCasualWeightPort(int counter, int totalOffer);

int createSharedMemory(size_t size);

void initLotSemaphore(int lotLength, int index);



#endif