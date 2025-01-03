#include <sys/ipc.h>

// struktura komunikatu
struct bufor{
	long mtype;
	int mvalue;
};

key_t getFtokKey(int keyID) {
    key_t ftokKey = ftok(".", keyID);

    if (ftokKey == -1) {
        printf("Blad ftok (main)\n");
        exit(1);
    }

    return ftokKey;
}

int initializeMessageQueue(int keyID) {
    key_t ftokKey = getFtokKey(keyID);
    int msgID = msgget(ftokKey,IPC_CREAT|IPC_EXCL|0666);

    if (msgID == -1) {
        printf("Blad kolejki komunikatow\n"); 
        exit(1);
    }

    return msgID;
}

int initializeSharedMemory(int keyID, int shmSize) {
    key_t ftokKey = getFtokKey(keyID);
    int shmID = shmget(ftokKey, shmSize, IPC_CREAT|IPC_EXCL|0666);
    
    if (shmID == -1) {
        printf("Blad pamieci dzielonej\n");
        exit(1);
    }

    return shmID;
}

void sendMessage(int msgID, struct bufor* message) {
    if (msgsnd(msgID, message, sizeof(message->mvalue), 0) == -1) {
        printf("Blad wyslania komunikatu\n");
        exit(1);
    }
}

void receiveMessage(int msgID, struct bufor* message, int msgType) {
    if (msgrcv(msgID, message, sizeof(message->mvalue), msgType, 0)==-1) {
        printf("Blad odbioru komunikatu\n");
        exit(1);
   }
}