typedef struct {
    double x; 
    double y; 
} coordinate; 

typedef struct {
    int idGood; // tipo di merce 
    double amount; // quantità merce 
    int life;   // tempo di vita misurato in giorni 
} good; 

typedef struct { 
    good *request;
    good *offer; 
} warehouse; 

typedef struct {
    int pid; 
    coordinate* position; 
    warehouse inventory; 
    int sem_docks_id; 
} port; 

typedef struct {
    int pid; 
    coordinate* position;
    good *listGoods; 
    double totalCargo; //decrementa/incrementa ogni volta che carico/scarico una merce
} ship;

int init_var(); 

int initPort(); 

int initShip(); 

double distance(coordinate* positionX, coordinate* positionY);


