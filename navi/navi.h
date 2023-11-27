#ifndef _NAVI_H
#define _NAVI_H




void initShip(ship* shipList);

double distance(coordinate positionX, coordinate positionY);

int moveToPort(char type, lot* lots, int idGood, ship* ship);

int findPorts(ship* shipList);

int findRequestPort(int idGood, lot* lots, ship* shipList); 

int loadLot(lot* lots, int idGood, ship* shipList);

void unloadLot(lot* lots, ship* shipList);

void remove_ipcs();

void alarmHandler(int signum);
/* int checkEconomy(); */




#endif