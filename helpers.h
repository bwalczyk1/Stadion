// struktura komunikatu
struct msg {
	long mType;
	int mValue;
};

struct valueWithCounter {
    int value;
    int counter;
};

// struktura komunikatu rozszerzonego
struct msgCounter {
    long mType;
    struct valueWithCounter mValueWithCounter;
};

int initializeMessageQueue(int keyID);
int initializeSharedMemory(int keyID, int shmSize);
void sendMessage(int msgID, struct msg* message);
void sendMessageWithCounter(int msgID, struct msgCounter* messageWithCounter);
void receiveMessage(int msgID, struct msg* message, int msgType);
void receiveMessageWithCounter(int msgID, struct msgCounter* messageWithCounter, int msgType);
int allocateSem(int keyID, int number);
int freeSem(int semID, int number);
void initializeSem(int semID, int number, int val);
int waitSem(int semID, int number);
void signalSem(int semID, int number);
int getSemValue(int semID, int number);