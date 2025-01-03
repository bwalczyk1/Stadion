#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <time.h>

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

int main() {
    msgID = initializeMessageQueue('A');
    shmID = initializeSharedMemory('B', MY_SHM_SIZE * sizeof(int));

    int* pam = (int*) shmat(shmID, NULL, 0);

    // Przechowuje informacje z której jest drużyny
    srand(time(NULL));   // Initialization, should only be called once.
    int team = rand() % 2 + 1;

    // Czeka na zwykły komunikat od pracownika technicznego o wolnym miejscu na stanowisku
    receiveMessage(msgID, &message, MSG_TYPE_FAN);
    
    // Jeśli nie jest odpowiednie
    if (message.mvalue != 0 && message.mvalue != team) {
        // Wysyła dalej ten sam komunikat
        sendMessage(msgID, &message);
        // Ustawia swoją liczbe przepuszczonych na 1
        int passed = 1;

        // Zwiększa liczbę przepuszczających o 1
        if (pam[MY_SHM_INDEX_WAITING_NUMBER] == 0) {
            pam[MY_SHM_INDEX_WAITING_TEAM] = team;
        }

        pam[MY_SHM_INDEX_WAITING_NUMBER] = pam[MY_SHM_INDEX_WAITING_NUMBER] + 1;
        
        // Dopóki true
        while (1){
            // Czeka na komunikat dla puszczających
            receiveMessage(msgID, &message, MSG_TYPE_FAN_WAITING);
            
            // Jeśli jest odpowiedni, break
            if (message.mvalue == 0 || message.mvalue == team) {
                pam[MY_SHM_INDEX_WAITING_NUMBER] = pam[MY_SHM_INDEX_WAITING_NUMBER] - 1;

                break;
            }

            // Zwiększa liczbę przepuszczonych o 1
            passed += 1;
            
            // Jeśli liczba przepuszczonych >= 5, zaczyna wyświetla informację o frustracji i agresywnym zachowaniu
            if (passed >= 5) {
                // ...
            }
        }
    }
    
    // Ustawia się do stanowiska
    
    return 0;
}