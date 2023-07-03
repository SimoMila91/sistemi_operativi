#ifndef _NAVI_H
#define _NAVI_H




void initShip(ship* shipList);
double distance(coordinate* positionX, coordinate* positionY);
void moveToPort(char type, lot* lots, int idGood, ship* ship);
int findPorts(ship* shipList);
int findRequestPort(int idGood, lot* lots, ship* shipList); 
void loadLot(lot* lots, int idGood, ship* shipList);
void unloadLot(lot* lots, ship* shipList);




#endif