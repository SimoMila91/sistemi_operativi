#define TEST_ERROR    if (errno && errno != EINTR) {fprintf(stderr, \
					  "\x1b[31m%s at line %d: PID=%5d, Error %d: %s\n\x1b[37m", \
					  _FILE_,			\
					  _LINE_,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno)); \
					  errno=0;\
					  }

// stampa il tipo di errore che si è generato 