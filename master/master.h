typedef struct {
    double x; 
    double y; 
} coordinate; 

typedef struct {
    int id_ship;
    double value; 
    int available; 
    int type; // se 1 => ricevuta; se 0 => generata
} lot; 

typedef struct {
    int idGood; // tipo di merce 
    double amount; // quantità merce 
    double remains; // solo per la request 
    int requestBooked; //solo per la request
    int life;   // tempo di vita misurato in giorni 
    lot *lots; 
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
    int keyOffer; 
    int keyRequest; 
} ship;

typedef struct {
    int keyPortMemory; 
    coordinate* position; 
} database; 

typedef struct {
    int shipWithCargo; 
    int shipWithoutCargo; 
    int shipToDocks; 
    // merce per tipologia e stato 
    // per ogni porto merce presente, consegnata e ricevuta
    // quantità totale generata dal master di ogni merce e tra queste: 
    //  - quante scadute in porto 
    //  - quante scadute in mare 
    //  - quante rimaste ferme in un porto 
    //  - quante consegnate da qualche nave 
    // indicare il porto che ha offerto la quantità maggiore della merce 
    // indicare il porto che ha richiesto la quantità maggiore della merce 

} report; 

int init_var(); 

void getCasualWeight(double* offer);

int createSharedMemory(size_t size);



