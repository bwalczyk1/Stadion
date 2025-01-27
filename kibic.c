#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#include "settings.h"
#include "helpers.h"

void controlProcess();
void* childControl(void* childControlStructVoid);
void waitToEnter();
void waitToExit();
void waitAlone(int gate);
void waitWithChild(int gate);
void* childWait(void* gateVoid);

struct controlStruct {
    int controlNumber;
    int isThreat;
    int result;
};

struct msg message;

int shmID, msgFanID, msgWorkerID, msgControlID, semID;  // ID kolejek kom., pamieci dzielonej i semaforów
int team, hasChild, isVip; // zmienne osobiste
pthread_t child;

int main() {
    msgFanID = initializeMessageQueue(MSG_QUEUE_FAN);
    msgWorkerID = initializeMessageQueue(MSG_QUEUE_WORKER);
    msgControlID = initializeMessageQueue(MSG_QUEUE_CONTROL);
    semID = allocateSem(SEM_KEY_ID, SEM_NUMBER);
    shmID = initializeSharedMemory(SHM_KEY_ID, (SHM_SIZE_WITHOUT_PLACES + PLACES) * sizeof(int));

    // Przechowuje informacje o sobie
    srand(getpid());
    team = (rand() % 100 < TEAM_1_CHANCE) + 1;
    hasChild = rand() % 100 < CHILD_CHANCE;
    isVip = rand() % 1000 < VIP_PER_MILLE;

    if (!isVip) {
        controlProcess();
    }

    // Wchodzi na stadion
    waitToEnter();
    printf("Kibic %d wszedł na stadion\n", getpid());
    printf("Sem: wejscie %d, wyjscie %d\n", getSemValue(semID, SEM_ENTRANCE_CONTROL), getSemValue(semID, SEM_EXIT_CONTROL));
    waitToExit();
    printf("Kibic %d wyszedł ze stadionu\n", getpid());
    printf("Sem: wejscie %d, wyjscie %d\n", getSemValue(semID, SEM_ENTRANCE_CONTROL), getSemValue(semID, SEM_EXIT_CONTROL));

    exit(0);
}

void controlProcess() {
    printf("Kibic %d czeka w kolejce\n", getpid());
    int* pam = (int*) shmat(shmID, NULL, 0);
    int isThreat = rand() % 100 < THREAT_CHANCE;
    int savedControlNumber = -1;
    struct controlStruct childControlStruct;
    childControlStruct.isThreat = hasChild ? (rand() % 100 < CHILD_THREAT_CHANCE) : 0;
    childControlStruct.controlNumber = -1;
    int passed = 0;

    // Czeka na zwykły komunikat od pracownika technicznego o wolnej kontroli
    receiveMessage(msgFanID, &message, MSG_EMPTY_CONTROL);

    int controlNumber = message.mValue;
    printf("Kibic %d otrzymal wiadomosc %d\n", getpid(), controlNumber);

    if (controlNumber == MSG_CONTROL_END) {
        sendMessage(msgFanID, &message);
        printf("Kibic %d przekazal wiadomosc %d dalej\n", getpid(), controlNumber);
        
        // Zakończ proces
        printf("Kibic %d opuscil kolejke\n", getpid());
        exit(0);
    }

    int messageTeam = pam[SHM_SIZE_WITHOUT_PLACES + controlNumber / PLACES];
    printf("Druzyna kibica %d: %d\n", getpid(), team);
    printf("Druzyna otrzymana przez kibica %d: %d\n", getpid(), messageTeam);

    if (messageTeam == 0 || messageTeam == team) {
        savedControlNumber = controlNumber;
        pam[SHM_SIZE_WITHOUT_PLACES + controlNumber / PLACES] = team;
    } else {
        // Wysyła dalej ten sam komunikat
        sendMessage(msgFanID, &message);
        printf("Kibic %d przekazal wiadomosc %d dalej\n", getpid(), controlNumber);
        // Ustawia swoją liczbe przepuszczonych na 1
        passed = 1;
    }

    if (hasChild || savedControlNumber == -1) {
        // Zwiększa liczbę przepuszczających o 1
        pam[SHM_INDEX_WAITING_NUMBER] = pam[SHM_INDEX_WAITING_NUMBER] + 1;
    }

    struct msgCounter messageWithCounter;

    if (hasChild) {
        printf("Kibic %d ma dziecko\n", getpid());
    }

    // Dopóki potrzebuje miejsca w kontoli
    while (savedControlNumber == -1 || hasChild && childControlStruct.controlNumber == -1){
        printf("Kibic %d czeka w kolejce dla puszczajacych\n", getpid());
        // Czeka na komunikat dla puszczających
        receiveMessageWithCounter(msgFanID, &messageWithCounter, MSG_EMPTY_CONTROL_FOR_WAITING);

        controlNumber = messageWithCounter.mValueWithCounter.value;
        printf("Kibic %d otrzymal wiadomosc %d\n", getpid(), controlNumber);

        if (controlNumber >= 0) {
            if (savedControlNumber == -1) {
                messageTeam = pam[SHM_SIZE_WITHOUT_PLACES + controlNumber / PLACES];
                printf("Druzyna kibica %d: %d\n", getpid(), team);
                printf("Druzyna otrzymana przez kibica %d: %d\n", getpid(), messageTeam);

                if (messageTeam == 0 || messageTeam == team) {
                    savedControlNumber = controlNumber;
                    pam[SHM_SIZE_WITHOUT_PLACES + controlNumber / PLACES] = team;
                    messageWithCounter.mValueWithCounter.value = MSG_CONTROL_TAKEN;
                }
            } else if (hasChild && savedControlNumber/PLACES == controlNumber/PLACES) {
                childControlStruct.controlNumber = controlNumber;
                messageWithCounter.mValueWithCounter.value = MSG_CONTROL_TAKEN;
            }

            if (messageWithCounter.mValueWithCounter.value != MSG_CONTROL_TAKEN) {
                // Zwiększa liczbę przepuszczonych o 1
                passed += 1;

                // Jeśli liczba przepuszczonych osiągnie 5, wyświetla informację o frustracji
                if (passed == 5) {
                    printf("Kibic %d jest sfrustrowany\n", getpid());
                }
            } else if (savedControlNumber >= 0 && (!hasChild || childControlStruct.controlNumber >= 0)) {
                pam[SHM_INDEX_WAITING_NUMBER] = pam[SHM_INDEX_WAITING_NUMBER] - 1;
            }
        }

        messageWithCounter.mValueWithCounter.counter = messageWithCounter.mValueWithCounter.counter - 1;

        if (messageWithCounter.mValueWithCounter.counter > 0) {
            sendMessageWithCounter(msgFanID, &messageWithCounter);
            printf("Kibic %d przekazal wiadomosc %d dalej\n", getpid(), messageWithCounter.mValueWithCounter.value);
        } else if (messageWithCounter.mValueWithCounter.value != MSG_CONTROL_TAKEN) {
            message.mValue = messageWithCounter.mValueWithCounter.value;
            sendMessage(msgFanID, &message);
            printf("Kibic %d przekazal wiadomosc %d dalej\n", getpid(), messageWithCounter.mValueWithCounter.value);
        }

        if (messageWithCounter.mValueWithCounter.value == MSG_CONTROL_END) {
            //Zakończ proces
            printf("Kibic %d opuscil kolejke\n", getpid());
            exit(0);
        }
    }

    // Ustawia się do stanowiska
    printf("Kibic %d przechodzi kontrole %d\n", getpid(), savedControlNumber);
    
    if (hasChild) {
        pthread_create(&child, NULL, &childControl, (void*) &childControlStruct);
    }

    message.mType = MSG_QUEUE_CONTROL_TYPES + savedControlNumber + 1;
    message.mValue = isThreat;
    sendMessage(msgControlID, &message);
    receiveMessage(msgFanID, &message, MSG_QUEUE_FAN_TYPES + savedControlNumber + 1);

    if (message.mValue == 0) {
        printf("Kibic %d nie przeszedl kontroli %d\n", getpid(), savedControlNumber);
        exit(0);
    }

    if (hasChild) {
        pthread_join(child, NULL);

        if (childControlStruct.result == 0) {
            printf("Kibic %d nie przeszedl kontroli %d\n", getpid(), childControlStruct.controlNumber);
            exit(0);
        }
    }
}

void* childControl(void* childControlStructVoid) {
    struct controlStruct* childControlStruct = (struct controlStruct*) childControlStructVoid;
    struct msg childMessage;
    childMessage.mType = MSG_QUEUE_CONTROL_TYPES + childControlStruct->controlNumber + 1;
    childMessage.mValue = childControlStruct->isThreat;

    sendMessage(msgControlID, &childMessage);
    receiveMessage(msgFanID, &childMessage, MSG_QUEUE_FAN_TYPES + childControlStruct->controlNumber + 1);

    childControlStruct->result = childMessage.mValue;
}

void waitToEnter() {
    int gate = SEM_ENTRANCE_CONTROL;

    if (hasChild) {
        waitWithChild(gate);
    } else {
        waitAlone(gate);
    }
    
    gate = isVip ? SEM_ENTRANCE_VIP : SEM_ENTRANCE;

    if (hasChild) {
        waitWithChild(gate);
    } else {
        waitAlone(gate);
    }
}

void waitToExit() {
    int gate = isVip ? SEM_EXIT_VIP : SEM_EXIT;

    if (hasChild) {
        waitWithChild(gate);
    } else {
        waitAlone(gate);
    }
    
    gate = SEM_EXIT_CONTROL;

    if (hasChild) {
        waitWithChild(gate);
    } else {
        waitAlone(gate);
    }
}

void waitAlone(int gate) {
    waitSem(semID, gate);

    if (gate != SEM_ENTRANCE_CONTROL && gate != SEM_EXIT_CONTROL) {
        signalSem(semID, gate);
    } else {
        signalSem(semID, gate == SEM_ENTRANCE_CONTROL ? SEM_EXIT_CONTROL : SEM_ENTRANCE_CONTROL);
    }
        
    if (gate == SEM_EXIT_CONTROL && getSemValue(semID, SEM_EXIT_CONTROL) == 0) {
        // Wyślij komunikat o opuszczeniu stadionu przez wszystkich
        message.mType = MSG_FANS_LEFT;
        message.mValue = 0;
        sendMessage(msgWorkerID, &message);
    }
}

void waitWithChild(int gate) {
    if (hasChild) {
        pthread_create(&child, NULL, &childWait, (void*) &gate);
    }

    waitAlone(gate);

    if (hasChild) {
        pthread_join(child, NULL);
    }
}

void* childWait(void* gateVoid) {
    int* gate = (int*) gateVoid;
    waitSem(semID, *gate);

    if (*gate != SEM_ENTRANCE_CONTROL && *gate != SEM_EXIT_CONTROL) {
        signalSem(semID, *gate);
    } else {
        signalSem(semID, *gate == SEM_ENTRANCE_CONTROL ? SEM_EXIT_CONTROL : SEM_ENTRANCE_CONTROL);
    }

    if (*gate == SEM_EXIT_CONTROL && getSemValue(semID, SEM_EXIT_CONTROL) == 0) {
        // Wyślij komunikat o opuszczeniu stadionu przez wszystkich
        message.mType = MSG_FANS_LEFT;
        message.mValue = 0;
        sendMessage(msgWorkerID, &message);
    }
}