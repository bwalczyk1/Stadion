#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <time.h>
#include <pthread.h>

#include <settings.h>
#include <helpers.h>

struct controlStruct {
    int controlNumber;
    int isThreat;
    int result;
};

struct msg message;

int shmID, msgFanID, msgControlID;  //ID kolejek kom., pamieci dzielonej

int main() {
    msgFanID = initializeMessageQueue(MSG_QUEUE_FAN);
    msgControlID = initializeMessageQueue(MSG_QUEUE_CONTROL);
    shmID = initializeSharedMemory('B', (SHM_SIZE_WITHOUT_PLACES + PLACES) * sizeof(int));

    int* pam = (int*) shmat(shmID, NULL, 0);

    // Przechowuje informacje o sobie
    srand(time(NULL));
    int team = rand() % 2 + 1;
    int isThreat = rand() % 2 + 1;
    int savedControlNumber = 0;
    int hasChild = rand() % 2 + 1;
    pthread_t child;
    struct controlStruct childControlStruct;
    childControlStruct.isThreat = hasChild ? (rand() % 2 + 1) : 0;
    childControlStruct.controlNumber = 0;
    int passed = 0;

    // Czeka na zwykły komunikat od pracownika technicznego o wolnej kontroli
    receiveMessage(msgFanID, &message, MSG_EMPTY_CONTROL);

    int controlNumber = message.mValue;
    int messageTeam = pam[SHM_SIZE_WITHOUT_PLACES + controlNumber / PLACES];

    if (messageTeam == 0 || messageTeam == team) {
        savedControlNumber = controlNumber;
        pam[SHM_SIZE_WITHOUT_PLACES + controlNumber / PLACES] = team;
    } else {
        // Wysyła dalej ten sam komunikat
        sendMessage(msgFanID, &message);
        // Ustawia swoją liczbe przepuszczonych na 1
        passed = 1;
    }

    if (hasChild || savedControlNumber == 0) {
        // Zwiększa liczbę przepuszczających o 1
        pam[SHM_INDEX_WAITING_NUMBER] = pam[SHM_INDEX_WAITING_NUMBER] + 1;
    }

    struct msgCounter messageWithCounter;

    // Dopóki potrzebuje miejsca w kontoli
    while (savedControlNumber == 0 || hasChild && childControlStruct.controlNumber == 0){
        // Czeka na komunikat dla puszczających
        receiveMessageWithCounter(msgFanID, &messageWithCounter, MSG_EMPTY_CONTROL_FOR_WAITING);

        controlNumber = messageWithCounter.mValueWithCounter.value;

        if (controlNumber >= 0) {
            if (savedControlNumber == 0) {
                messageTeam = pam[SHM_SIZE_WITHOUT_PLACES + controlNumber / PLACES];

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

                // Jeśli liczba przepuszczonych >= 5, zaczyna wyświetla informację o frustracji i agresywnym zachowaniu
                if (passed >= 5) {
                    // ...
                }
            } else if (savedControlNumber && (!hasChild || childControlStruct.controlNumber)) {
                pam[SHM_INDEX_WAITING_NUMBER] = pam[SHM_INDEX_WAITING_NUMBER] - 1;
            }
        }

        messageWithCounter.mValueWithCounter.counter = messageWithCounter.mValueWithCounter.counter - 1;

        if (messageWithCounter.mValueWithCounter.counter > 0) {
            sendMessageWithCounter(msgFanID, &messageWithCounter);
        } else if (messageWithCounter.mValueWithCounter.value != MSG_CONTROL_TAKEN) {
            message.mValue = messageWithCounter.mValueWithCounter.value;
            sendMessage(msgFanID, &message);
        }
    }

    // Ustawia się do stanowiska
    
    if (hasChild) {
        pthread_create(&child, NULL, &childControl, (void*) &childControlStruct);
    }

    message.mType = savedControlNumber;
    message.mValue = isThreat;
    sendMessage(msgControlID, &message);
    receiveMessage(msgFanID, &message, savedControlNumber);

    if (message.mValue == 0) {
        return 1;
    }

    if (hasChild) {
        pthread_join(child, NULL);

        if (childControlStruct.result == 0) {
            return 1;
        }
    }

    // Wchodzi na stadion
    // ...

    return 0;
}

void* childControl(void* childControlStructVoid) {
    struct controlStruct* childControlStruct = (struct controlStruct*) childControlStructVoid;
    struct msg childMessage;
    childMessage.mType = childControlStruct->controlNumber;
    childMessage.mValue = childControlStruct->isThreat;

    sendMessage(msgControlID, &childMessage);
    receiveMessage(msgFanID, &childMessage, childControlStruct->controlNumber);

    childControlStruct->result = childMessage.mValue;
}