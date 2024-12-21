#include <sys/types.h>
#include <signal.h>

int main() {
    // Odbiera komunikat o PID pracownika
    int pracownikPID;
    // Wysyła do pracownika sygnał1, żeby wstrzymać wpuszczanie kibiców'
    kill(pracownikPID, SIGUSR1);
    // Wysyła do pracownika sygnał2, żeby wznowić wpuszczanie kibiców
    kill(pracownikPID, SIGUSR1);
    // Wysyła do pracownika sygnał3, żeby kibice opuścili stadion
    kill(pracownikPID, SIGINT);
    // Odbiera informacje, że wszyscy kibice opuścili stadion
    
    return 0;
}