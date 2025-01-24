#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "settings.h"

int main() {
    srand(getpid());

    // Włącza pracownika technicznego, kierownika i kibiców
    if (fork() == 0) {
        execl("./pracownik_techniczny", "pracownik_techniczny", NULL);
    }
    
    if (fork() == 0) {
        execl("./kierownik", "kierownik", NULL);
    }

    for (int i = 0; i < FANS; i++) {
        usleep((rand() % 2000 + 1000) * 1000);

        if (fork() == 0) {
            execl("./kibic", "kibic", NULL);
        }
    }

    while (1);
}