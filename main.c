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
        sleep(rand() % 5 + 1);

        if (fork() == 0) {
            execl("./kibic", "kibic", NULL);
        }
    }

    return 0;
}