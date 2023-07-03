#ifndef _PORTI_H
#define _PORTI_H


void initPort(int i, int totalOffer, int totalRequest, int semId);

double distance(coordinate* positionX, coordinate* positionY);

int isDuplicate(int numGood, int numOffer);

void initializeInventory(int totalOffer, int totalRequest);

void initDocksSemaphore();

lot* createLoots(double amount, int index);

int* getCasualWeightPort(int counter, int totalOffer);

int createSharedMemory(size_t size);



#endif