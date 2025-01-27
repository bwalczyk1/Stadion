#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/errno.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h>

#include "settings.h"

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

key_t getFtokKey(int keyID) {
    key_t ftokKey = ftok(".", keyID);

    if (ftokKey == -1) {
        perror("Blad ftok ");
        exit(1);
    }

    return ftokKey;
}

int initializeMessageQueue(int keyID) {
    key_t ftokKey = getFtokKey(keyID);
    int msgID = msgget(ftokKey, IPC_CREAT | 0666);

    if (msgID == -1) {
        perror("Blad kolejki komunikatow: "); 
        exit(1);
    }

    return msgID;
}

int initializeSharedMemory(int keyID, int shmSize) {
    key_t ftokKey = getFtokKey(keyID);
    int shmID = shmget(ftokKey, shmSize, IPC_CREAT | 0666);
    
    if (shmID == -1) {
        perror("Blad pamieci dzielonej:");
        exit(1);
    }

    return shmID;
}

void sendMessage(int msgID, struct msg* message) {
    if (msgsnd(msgID, message, sizeof(message->mValue), 0) == -1) {
        perror("Blad wyslania komunikatu:");
        exit(1);
    }
}

void sendMessageWithCounter(int msgID, struct msgCounter* messageWithCounter) {
    if (msgsnd(msgID, messageWithCounter, sizeof(messageWithCounter->mValueWithCounter), 0) == -1) {
        perror("Blad wyslania komunikatu: ");
        exit(1);
    }
}

void receiveMessage(int msgID, struct msg* message, int msgType) {
    if (msgrcv(msgID, message, sizeof(message->mValue), msgType, 0) == -1) {
        perror("Blad odbioru komunikatu: ");
        exit(1);
    }
}

void receiveMessageWithCounter(int msgID, struct msgCounter* messageWithCounter, int msgType) {
    if (msgrcv(msgID, messageWithCounter, sizeof(messageWithCounter->mValueWithCounter), msgType, 0) == -1) {
        perror("Blad odbioru komunikatu: ");
        exit(1);
    }
}

int allocateSem(int keyID, int number)
{
    key_t ftokKey = getFtokKey(keyID);
    int semID= semget(ftokKey, number, IPC_CREAT | 0666);
    
    if (semID== -1) {
        perror("Blad semget: ");
        exit(1);
    }

    return semID;
}

void freeSem(int semID, int number)
{
    semctl(semID, number, IPC_RMID, NULL);
}

void initializeSem(int semID, int number, int val)
{
    if ( semctl(semID, number, SETVAL, val) == -1 )
    {
        perror("Blad semctl: ");
        exit(1);
    }
}

void waitSem(int semID, int number)
{
    struct sembuf operacje[1];
    operacje[0].sem_num = number;
    operacje[0].sem_op = -1;
    operacje[0].sem_flg = 0;;
    
    for (int i = 0; i < TRIES; i++) {
        if (semop(semID, operacje, 1) == -1) {
            perror("Blad semop");
            continue;
        }

        return;
    }
            
    exit(1);
}

void signalSem(int semID, int number)
{
    struct sembuf operacje[1];
    operacje[0].sem_num = number;
    operacje[0].sem_op = 1;

    for (int i = 0; i < TRIES; i++) {
        if (semop(semID, operacje, 1) == -1) {
            perror("Blad semop");
            continue;
        }

        return;
    }

    exit(1);
}

int getSemValue(int semID, int number)
{
    return semctl(semID, number, GETVAL, NULL);
}