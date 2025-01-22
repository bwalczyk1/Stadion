#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include "helpers.h"
#include "settings.h"


int main() {
    srand(time(NULL));
    // Odbiera komunikat o PID pracownika
    int msgBossID = initializeMessageQueue(MSG_QUEUE_BOSS);
    struct msg message;
    receiveMessage(msgBossID, &message, MSG_BOSS_INFO);
    int pracownikPID = message.mValue;

    // Wysyła do pracownika sygnał1, żeby wstrzymać wpuszczanie kibiców'
    sleep(rand() % 10 + 10);
    kill(pracownikPID, SIGUSR1);
    // Wysyła do pracownika sygnał2, żeby wznowić wpuszczanie kibiców
    sleep(rand() % 5 + 5);
    kill(pracownikPID, SIGUSR2);
    // Wysyła do pracownika sygnał3, żeby kibice opuścili stadion
    sleep(rand() % 20 + 20);
    kill(pracownikPID, SIGINT);
    // Odbiera informacje, że wszyscy kibice opuścili stadion
    receiveMessage(msgBossID, &message, MSG_BOSS_INFO);
    
    return 0;
}