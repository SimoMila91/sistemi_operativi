void initPort(int i, double totalOffer, double totalRequest, int semId);

double distance(coordinate* positionX, coordinate* positionY);

int isDuplicate(int numGood, int numOffer);

void initializeInventory(double totalOffer, double totalRequest);

void initDocksSemaphore();

lot* createLoots(double amount, int index);

void getCasualWeightPort(double offer[], int counter, double totalOffer);

int createSharedMemory(size_t size);

void init_var_port();