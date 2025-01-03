#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>

#include <helpers.h>

#define MY_SHM_INDEX_WAITING_TEAM 0
#define MY_SHM_INDEX_WAITING_NUMBER 0
#define MY_SHM_SIZE 1

#define MSG_TYPE_FAN 1
#define MSG_TYPE_FAN_WAITING 2

// struktura komunikatu
struct bufor{
	long mtype;
	int mvalue;
};

struct bufor message;

int shmID, msgID;  //ID kolejki kom., pamieci dzielonej

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
    msgID = initializeMessageQueue('A');
    shmID = initializeSharedMemory('B', MY_SHM_SIZE * sizeof(int));

    int* pam = (int*) shmat(shmID, NULL, 0);
    pam[MY_SHM_INDEX_WAITING_TEAM] = 0;
    pam[MY_SHM_INDEX_WAITING_NUMBER] = 0;

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

    // Komunikaty dla zwykłych kibiców i oczekujących mogą być na tej samej kolejce

    // Dopóki true
    while (1) {
        // Czeka na komunikat od stanowiska kontroli / kontroli
        int control = 1;
        message.mvalue = control;

        // Jeśli liczba przepuszczających > 0 i dostępna kontrola odpowiada przepuszczającym, wysyła jeden komunikat do przepuszczających i continue
        if (pam[MY_SHM_INDEX_WAITING_NUMBER] > 0 && (message.mvalue == 0 || message.mvalue == pam[MY_SHM_INDEX_WAITING_TEAM])) {
            message.mtype = MSG_TYPE_FAN_WAITING;
            send_message(msgID, &message);

            continue;
        }

        // Do każdego z przepuszczających wysyła komunikat
        message.mtype = MSG_TYPE_FAN_WAITING;
        
        for (int i = 0; i < pam[MY_SHM_INDEX_WAITING_NUMBER]; i++) {
            send_message(msgID, &message);
        }

        // Wysyła komunikat do zwykłych kibiców
        message.mtype = MSG_TYPE_FAN;
        send_message(msgID, &message);
    }

    // Kontroluje 3 stanowiska kontroli
    // Stanowiska kontroli to semafory, licznik odpowiada za liczbe wolnych miejsc ???
    // Kolejka osobnym programem ???
    // Pamięć dzielona, aby wiedziec, którzy kibice są w danej kolejce ???
    
    return 0;
}