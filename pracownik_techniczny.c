#include <signal.h>

void handler1() {

}

void handler2() {

}

void handler3() {
    // Wysyła do wszystkich 
}

int main() {
    // Inicjuje mechanizmy synchronizujące
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
    // Kontroluje 3 kolejki
    
    return 0;
}