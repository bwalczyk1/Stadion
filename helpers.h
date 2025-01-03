int initializeMessageQueue(int keyID);
int initializeSharedMemory(int keyID, int shmSize);
void sendMessage(int msgID, struct bufor* message);
void receiveMessage(int msgID, struct bufor* message, int msgType);