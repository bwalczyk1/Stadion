#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>

#include <settings.h>
#include <helpers.h>

struct msg message;
struct msgCounter messageWithCounter;

int shmID, msgFanID, msgWorkerID, msgControlID, semID;  // ID kolejek kom., pamieci dzielonej i semaforów
int* pam; // wskaźnik do pamięci dzielonej
int placesLeft; // liczba pozostałych miejsc na stadionie

void openEntrance() {
    if (placesLeft > 0){
        initializeSem(semID, SEM_ENTRANCE_CONTROL, placesLeft);
        placesLeft = 0;
    }
}

void closeEntrance() {
    if (placesLeft == 0){
        placesLeft = getSemValue(semID, SEM_ENTRANCE_CONTROL);
        initializeSem(semID, SEM_ENTRANCE_CONTROL, 0);
    }
}

void openExit() {
    // Rozpocznij wypuszczanie kibiców
    initializeSem(semID, SEM_EXIT_VIP, 1);
    initializeSem(semID, SEM_EXIT, 1);

    // Odpraw kibiców czekających do kontroli
    if (pam[SHM_INDEX_WAITING_NUMBER] > 0) {
        messageWithCounter.mType = MSG_EMPTY_CONTROL_FOR_WAITING;
        messageWithCounter.mValueWithCounter.value = MSG_CONTROL_END;
        messageWithCounter.mValueWithCounter.counter = pam[SHM_INDEX_WAITING_NUMBER];
        sendMessageWithCounter(msgFanID, &messageWithCounter);
    } else {
        message.mType = MSG_EMPTY_CONTROL;
        message.mValue = MSG_CONTROL_END;
        sendMessage(msgFanID, &message);
    }

    receiveMessage(msgFanID, &message, MSG_EMPTY_CONTROL);

    // Poczekaj aż wszyscy opuszczą stadion
    receiveMessage(msgWorkerID, &message, MSG_FANS_LEFT);

    // Wyłącz kontrole
    message.mValue = MSG_CONTROL_END;

    for (int i = 0; i < PLACES * CONTROLS_PER_PLACE; i++) {
        message.mType = MSG_QUEUE_CONTROL_TYPES + i + 1;
        sendMessage(msgControlID, &message);
    }

    // Zakończ wszystkie procesy synchronizacji
    for (int i = 0; i < SEM_NUMBER; i++){
        freeSem(semID, i);
    }

    shmdt(pam);
    shmctl(shmID, IPC_RMID, NULL);

    msgctl(msgFanID, IPC_RMID, NULL);
    msgctl(msgWorkerID, IPC_RMID, NULL);
    msgctl(msgControlID, IPC_RMID, NULL);
}

int main() {
    // Inicjuje mechanizmy synchronizujące
    msgFanID = initializeMessageQueue(MSG_QUEUE_FAN);
    msgWorkerID = initializeMessageQueue(MSG_QUEUE_WORKER);
    msgControlID = initializeMessageQueue(MSG_QUEUE_CONTROL);
    semID = allocateSem(SEM_KEY_ID, SEM_NUMBER);
    shmID = initializeSharedMemory(SHM_KEY_ID, (SHM_SIZE_WITHOUT_PLACES + PLACES) * sizeof(int));

    pam = (int*) shmat(shmID, NULL, 0);
    pam[SHM_INDEX_WAITING_NUMBER] = 0;

    for (int i = 0; i < PLACES; i++) {
        pam[SHM_SIZE_WITHOUT_PLACES + i] = 0;
    } 

    // Obsługuje sygnał1
    signal(SIGUSR1, closeEntrance);
    // Obsługuje sygnał2
    signal(SIGUSR2, openEntrance);
    // Obsługuje sygnał3
    signal(SIGINT, openExit);

    // Ustawia odpowiednie semafory
    initializeSem(semID, SEM_ENTRANCE_VIP, 1);
    initializeSem(semID, SEM_ENTRANCE, 1);

    placesLeft = K;
    openEntrance();

    for (int i = 0; i < PLACES * CONTROLS_PER_PLACE; i++) {
        // Włącza proces kontroli
        // ...

        // Wysyła kontroli jej numer
        message.mType = MSG_INITIATE_CONTROL;
        message.mValue = i;

        sendMessage(msgControlID, &message);
    }

    // Ustaw semafory

    // Dopóki true
    while (1) {
        // Czeka na komunikat od kontroli
        receiveMessage(msgWorkerID, &message, MSG_EMPTY_CONTROL);

        // Jeśli liczba przepuszczających > 0 wysyła do przepuszczających i continue
        if (pam[SHM_INDEX_WAITING_NUMBER] > 0 ) {
            messageWithCounter.mType = MSG_EMPTY_CONTROL_FOR_WAITING;
            messageWithCounter.mValueWithCounter.value = message.mValue;
            messageWithCounter.mValueWithCounter.counter = pam[SHM_INDEX_WAITING_NUMBER];
            sendMessageWithCounter(msgFanID, &messageWithCounter);

            continue;
        }

        // Wysyła komunikat do zwykłych kibiców
        send_message(msgFanID, &message);
    }

    // Kontroluje 3 stanowiska kontroli
    // Kolejka osobnym programem
    
    return 0;
}