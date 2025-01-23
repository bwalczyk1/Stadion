#include <stdlib.h>
#include <stdio.h>
#include "helpers.h"
#include "settings.h"

int msgFanID, msgWorkerID, msgControlID;  //ID kolejek komentarzy

struct msg message;

int main() {
    msgFanID = initializeMessageQueue(MSG_QUEUE_FAN);
    msgWorkerID = initializeMessageQueue(MSG_QUEUE_WORKER);
    msgControlID = initializeMessageQueue(MSG_QUEUE_CONTROL);

    // Czeka na komunikat o swoim numerze
    receiveMessage(msgControlID, &message, MSG_INITIATE_CONTROL);

    int controlNumber = message.mValue;
    printf("Kontrola %d wlaczona\n", controlNumber);

    // Wysyła komunikat o wolnym miejscu
    message.mType = MSG_EMPTY_CONTROL;
    sendMessage(msgWorkerID, &message);

    printf("Kontrola %d wyslala wiadomosc\n", controlNumber);

    while (1) {
        // Czeka na komunikat od kibica
        receiveMessage(msgControlID, &message, MSG_QUEUE_CONTROL_TYPES + controlNumber + 1);

        if (message.mValue == MSG_CONTROL_END) {
            // Zakończ proces
            exit(0);
        } else if (message.mValue) {
            // Kibic nie przeszedł kontroli
            message.mValue = 0;
        } else {
            // Kibic przeszedł kontrole
            message.mValue = 1;
        }

        // Wysyła komunikat zwrotny kibicowi
        message.mType = MSG_QUEUE_FAN_TYPES + controlNumber + 1;
        sendMessage(msgFanID, &message);

        // Wysyła komunikat o ponownie wolnym miejscu
        message.mType = MSG_EMPTY_CONTROL;
        message.mValue = controlNumber;
        sendMessage(msgWorkerID, &message);
        printf("Kontrola %d wyslala wiadomosc\n", controlNumber);
    }

    return 0;
}