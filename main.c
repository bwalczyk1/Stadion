#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "settings.h"

int main() {
    srand(time(NULL));

    // Włącza pracownika technicznego, kierownika i K kibiców
    if (fork() == 0) {
        execl("./pracownik_techniczny", "pracownik_techniczny", NULL);
    }
    
    if (fork() == 0) {
        execl("./kierownik", "kierownik", NULL);
    }

    for (int i = 0; i < K; i++) {
        usleep((rand() % 2000 + 1000) * 1000);

        if (fork() == 0) {
            execl("./kibic", "kibic", NULL);
        }
    }

    while (1);
}