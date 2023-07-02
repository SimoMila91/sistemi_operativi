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

// stampa il tipo di errore che si è generato 


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


#endif			  