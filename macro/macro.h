#ifndef _MACRO_H
#define _MACRO_H

#define TEST_ERROR    if (errno && errno != EINTR) {fprintf(stderr, \
					  "\x1b[31m%s at line %d: PID=%5d, Error %d: %s\n\x1b[37m", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno)); \
					  errno=0;\
					  }

/* stampa il tipo di errore che si è generato */


#define SO_PORTI atoi(getenv("SO_PORTI"))
#define SO_NAVI atoi(getenv("SO_NAVI"))
#define SO_LATO atoi(getenv("SO_LATO"))
#define SO_BANCHINE atoi(getenv("SO_BANCHINE")) 
#define SO_MERCI atoi(getenv("SO_MERCI"))
#define SO_SIZE atoi(getenv("SO_SIZE"))
#define SO_MAX_VITA atoi(getenv("SO_MAX_VITA")) 
#define SO_MIN_VITA atoi(getenv("SO_MIN_VITA")) 
#define SO_FILL atoi(getenv("SO_FILL"))
#define SO_DAYS atoi(getenv("SO_DAYS"))
#define SO_SPEED atoi(getenv("SO_SPEED"))	


typedef struct {
    double x; 
    double y; 
} coordinate; 

typedef struct {
    int id_ship;
    int value; 
    int available; 
    int type; // se 1 => ricevuta; se 0 => generata
} lot; 

typedef struct {
    int idGood; // tipo di merce 
    int amount; // quantità merce 
    int remains; // solo per la request 
    int requestBooked; //solo per la request
    int life;   // tempo di vita misurato in giorni 
    lot *lots; 
    int maxLoots;
} good; 

typedef struct { 
    good request;
    good *offer; 
    int counterGoodsOffer; /// quanti tipi di merce offro
} warehouse; 

typedef struct {
    int pid; 
    coordinate position; 
    warehouse inventory; 
    int sem_docks_id; 
    int sem_inventory_id;
} port; 

typedef struct {
    int pid; 
    coordinate position;
    good listGoods; 
    int keyOffer; 
    int keyRequest; 
} ship;

typedef struct {
    int keyPortMemory; 
    coordinate position; 
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

#endif			  
