#ifndef _MACRO_H
#define _MACRO_H

/* stampa il tipo di errore che si è generato */

#define TEST_ERROR    if (errno && errno != EINTR) {fprintf(stderr, \
					  "\x1b[31m%s at line %d: PID=%5d, Error %d: %s\n\x1b[37m", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno)); \
					  errno=0;\
					  }

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
    int idGood;  /* tipo di merce */  
    int id_ship;
    int life;    /* tempo di vita misurato in giorni */ 
    int value; 
    int available; 
    int status; /* se 0 nel porto, se 1 in nave, se 2 consegnato */
} lot; 

typedef struct {
    int idGood; 
    int amount;         /*  quantità merce */
    int remains;        /* solo per la request */ 
    int life; 
    int requestBooked;  /* solo per la request */
    int keyLots;   /** chiave memoria condivisa che contiene lotti dell'offerta */ 
    int semLot; 
    int maxLoots;
} good; 

typedef struct { 
    good request;
    int keyOffers; /** chiave memoria condivisa che contiene tutte le offerte */ 
    int counterGoodsOffer; /* quanti tipi di merce offro */ 
} warehouse; 

typedef struct {
    int pid; 
    coordinate position; 
    warehouse inventory; 
    int sem_docks_id; /* semaforo banchine */
    int totalDocks; /* totale banchine */
    int sem_inventory_id;
} port; 

typedef struct {
    int pid; 
    coordinate position;
    good listGoods; 
    int keyOffer; 
    int keyRequest; 
    int keyLot; 
    int statusCargo; /* 0 = senza merce a bordo,  1 = con merce a bordo */
    int statusPosition; /* 0 = in mare, 1 = carico/scarico */
} ship;

typedef struct {
    int keyPortMemory; 
    coordinate position; 
} database; 

#endif			  
