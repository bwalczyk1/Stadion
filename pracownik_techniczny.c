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

void handler1() {

}

void handler2() {

}

void handler3() {
    // Wysyła do wszystkich 
    // Wyjście zwykłe i VIP - semafory, pierwsze otwarcia tu, potem przekazują sobie nazwajem 
}

int main() {
    // Inicjuje mechanizmy synchronizujące
    msgFanID = initializeMessageQueue(MSG_QUEUE_FAN);
    msgWorkerID = initializeMessageQueue(MSG_QUEUE_WORKER);
    msgControlID = initializeMessageQueue(MSG_QUEUE_CONTROL);
    semID = allocateSem(SEM_KEY_ID, SEM_NUMBER);
    shmID = initializeSharedMemory(SHM_KEY_ID, (SHM_SIZE_WITHOUT_PLACES + PLACES) * sizeof(int));

    int* pam = (int*) shmat(shmID, NULL, 0);
    pam[SHM_INDEX_WAITING_NUMBER] = 0;

    for (int i = 0; i < PLACES; i++) {
        pam[SHM_SIZE_WITHOUT_PLACES + i] = 0;
    } 

    // Obsługuje sygnał1
    struct sigaction act1;
    act1.sa_handler=handler1;
    sigemptyset(&act1.sa_mask);
    act1.sa_flags=0;
    sigaction(SIGUSR1,&act1,0);
    // Obsługuje sygnał2
    struct sigaction act2;
    act2.sa_handler=handler2;
    sigemptyset(&act2.sa_mask);
    act2.sa_flags=0;
    sigaction(SIGUSR2,&act2,0);
    // Obsługuje sygnał3
    struct sigaction act3;
    act3.sa_handler=handler3;
    sigemptyset(&act3.sa_mask);
    act3.sa_flags=0;
    sigaction(SIGINT,&act3,0);

    // Ustawia odpowiednie semafory
    initializeSem(semID, SEM_ENTRANCE_CONTROL, K);
    initializeSem(semID, SEM_ENTRANCE_VIP, 1);
    initializeSem(semID, SEM_ENTRANCE, 1);

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
            messageWithCounter.mValueWithCounter.counter = SHM_INDEX_WAITING_NUMBER;
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