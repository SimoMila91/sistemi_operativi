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
    int freeDocks; 
} port; 

typedef struct {
    int pid; 
    coordinate* position;
    good *listGoods; 
    double totalCargo; //decrementa/incrementa ogni volta che carico/scarico una merce
} nave;

int init_var(); 

int spawnPorts(); 

int spawnShips(); 



