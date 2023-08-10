#ifndef _MASTER_H
#define _MASTER_H


int* getCasualWeight();

void alarmHandler(int signum);

void dumpSimulation(int type);

void initializeMatrix(int matrix[][], int lenght); 

void printTotalGoods(int matrix[][], int lenght); 

void printReportPorts(int matrix[][], int lenght);

void printGoodsReport(int matrix[][], int lenght);

void printMaxPort(int matrixOffer[][], int matrixRequest[][], int lenght);

#endif

