#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <unistd.h>

#include "settings.h"
#include "helpers.h"

struct msg message;
struct msgCounter messageWithCounter;

int shmID, msgFanID, msgWorkerID, msgControlID, msgBossID, semID;  // ID kolejek kom., pamieci dzielonej i semaforów
int* pam; // wskaźnik do pamięci dzielonej
int placesLeft; // liczba pozostałych miejsc na stadionie

void waitForEmptyControl();

void openEntrance() {
    if (placesLeft > 0){
        printf("Wejście otwarte\n");
        initializeSem(semID, SEM_ENTRANCE_CONTROL, placesLeft);
        placesLeft = 0;
    }

    waitForEmptyControl();
}

void closeEntrance() {
    if (placesLeft == 0){
        printf("Wejście zamknięte\n");
        placesLeft = getSemValue(semID, SEM_ENTRANCE_CONTROL);
        initializeSem(semID, SEM_ENTRANCE_CONTROL, 0);
    }

    waitForEmptyControl();
}

void openExit() {
    printf("Wpuszczanie zakonczone\n");

    // Odpraw kibiców czekających do kontroli
    if (pam[SHM_INDEX_WAITING_NUMBER] > 0) {
        messageWithCounter.mType = MSG_EMPTY_CONTROL_FOR_WAITING;
        messageWithCounter.mValueWithCounter.value = MSG_CONTROL_END;
        messageWithCounter.mValueWithCounter.counter = pam[SHM_INDEX_WAITING_NUMBER];
        sendMessageWithCounter(msgFanID, &messageWithCounter);
        printf("Pracownik techniczny wyslal wiadomosc do %d czekajacych\n", pam[SHM_INDEX_WAITING_NUMBER]);
    } else {
        message.mType = MSG_EMPTY_CONTROL;
        message.mValue = MSG_CONTROL_END;
        sendMessage(msgFanID, &message);
        printf("Pracownik techniczny wyslal wiadomosc\n");
    }

    while (1) {
        printf("Pracownik techniczny czeka na wiadomosc\n");
        receiveMessage(msgFanID, &message, MSG_EMPTY_CONTROL);
        printf("Pracownik techniczny otrzymal wiadomosc\n");

        if (message.mValue == MSG_CONTROL_END) {
            break;
        }
    }

    // Jeśli na stadionie są jacyś kibice 
    if (getSemValue(semID, SEM_EXIT_CONTROL) > 0){
        // Rozpocznij wypuszczanie kibiców
        initializeSem(semID, SEM_EXIT_VIP, 1);
        initializeSem(semID, SEM_EXIT, 1);

        printf("Pracownik techniczny czeka na wiadomosc\n");

        // while(1) {
            receiveMessage(msgWorkerID, &message, MSG_FANS_LEFT);
        // }

        printf("Pracownik techniczny otrzymal wiadomosc\n");
    }

    message.mType = MSG_BOSS_INFO;
    message.mValue = MSG_CONTROL_END;
    sendMessage(msgBossID, &message);

    // Wyłącz kontrole
    printf("Pracownik techniczny wylacza kontrole\n");
    message.mValue = MSG_CONTROL_END;
    
    for (int i = 0; i < PLACES * CONTROLS_PER_PLACE; i++) {
        message.mType = MSG_QUEUE_CONTROL_TYPES + i + 1;
        sendMessage(msgControlID, &message);
    }

    printf("Pracownik techniczny wylaczyl kontrole\n");

    // Zakończ wszystkie procesy synchronizacji
    for (int i = 0; i < SEM_NUMBER; i++){
        freeSem(semID, i);
    }

    shmdt(pam);
    shmctl(shmID, IPC_RMID, NULL);

    msgctl(msgFanID, IPC_RMID, NULL);
    msgctl(msgWorkerID, IPC_RMID, NULL);
    msgctl(msgBossID, IPC_RMID, NULL);
    msgctl(msgControlID, IPC_RMID, NULL);
    kill(getppid(), SIGKILL);
    exit(0);
}

int main() {
    printf("Pracownik techniczny %d rozpoczął pracę.\n", getpid());
    // Inicjuje mechanizmy synchronizujące
    msgFanID = initializeMessageQueue(MSG_QUEUE_FAN);
    msgWorkerID = initializeMessageQueue(MSG_QUEUE_WORKER);
    msgControlID = initializeMessageQueue(MSG_QUEUE_CONTROL);
    msgBossID = initializeMessageQueue(MSG_QUEUE_BOSS);
    semID = allocateSem(SEM_KEY_ID, SEM_NUMBER);
    shmID = initializeSharedMemory(SHM_KEY_ID, (SHM_SIZE_WITHOUT_PLACES + PLACES) * sizeof(int));

    pam = (int*) shmat(shmID, NULL, 0);
    pam[SHM_INDEX_WAITING_NUMBER] = 0;

    for (int i = 0; i < PLACES; i++) {
        pam[SHM_SIZE_WITHOUT_PLACES + i] = i % 3;
    } 

    // Obsługuje sygnał1
    signal(SIGUSR1, closeEntrance);
    // Obsługuje sygnał2
    signal(SIGUSR2, openEntrance);
    // Obsługuje sygnał3
    signal(SIGINT, openExit);

    // Wysyła swoje PID do kierownika
    message.mType = MSG_BOSS_INFO;
    message.mValue = getpid();
    sendMessage(msgBossID, &message);

    // Ustawia odpowiednie semafory
    initializeSem(semID, SEM_EXIT_CONTROL, 0);
    initializeSem(semID, SEM_EXIT_VIP, 0);
    initializeSem(semID, SEM_EXIT, 0);
    initializeSem(semID, SEM_ENTRANCE_VIP, 1);
    initializeSem(semID, SEM_ENTRANCE, 1);


    for (int i = 0; i < PLACES * CONTROLS_PER_PLACE; i++) {
        // Włącza proces kontroli
        if (fork() == 0) {
            execl("./kontrola", "kontrola", NULL);
        }

        // Wysyła kontroli jej numer
        message.mType = MSG_INITIATE_CONTROL;
        message.mValue = i;

        sendMessage(msgControlID, &message);
    }

    placesLeft = K;
    openEntrance();

    return 0;
}

void waitForEmptyControl() {
    // Dopóki true
    while (1) {
        // Czeka na komunikat od kontroli
        printf("Pracownik techniczny czeka na wiadomosc\n");
        receiveMessage(msgWorkerID, &message, MSG_EMPTY_CONTROL);
        printf("Pracownik techniczny otrzymal wiadomosc\n");


        // Jeśli liczba przepuszczających > 0 wysyła do przepuszczających i continue
        if (pam[SHM_INDEX_WAITING_NUMBER] > 0 ) {
            messageWithCounter.mType = MSG_EMPTY_CONTROL_FOR_WAITING;
            messageWithCounter.mValueWithCounter.value = message.mValue;
            messageWithCounter.mValueWithCounter.counter = pam[SHM_INDEX_WAITING_NUMBER];
            sendMessageWithCounter(msgFanID, &messageWithCounter);
            printf("Pracownik techniczny wyslal wiadomosc do %d czekajacych\n", pam[SHM_INDEX_WAITING_NUMBER]);

            continue;
        }

        // Wysyła komunikat do zwykłych kibiców
        sendMessage(msgFanID, &message);
        printf("Pracownik techniczny wyslal wiadomosc\n");
    }
}