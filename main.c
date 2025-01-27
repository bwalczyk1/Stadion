#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>

#include "settings.h"

void* waitForChild();
void finishWorking();
pthread_t waitingThreads[FANS + 2];

int main() {
    signal(SIGINT, finishWorking);
    srand(getpid());

    // Włącza pracownika technicznego, kierownika i kibiców
    switch (fork()) {
        case -1:
            perror("Blad wydzielenia procesu");
            exit(1);
        case 0:
            if (execl("./pracownik_techniczny", "pracownik_techniczny", NULL) == -1) {
                perror("Blad wywolania procesu");
                exit(1);
            }
    }

    pthread_create(waitingThreads, NULL, &waitForChild, NULL);

    switch (fork()) {
        case -1:
            perror("Blad wydzielenia procesu");
            exit(1);
        case 0:
            if (execl("./kierownik", "kierownik", NULL) == -1) {
                perror("Blad wywolania procesu");
                exit(1);
            }
    }

    pthread_create(waitingThreads + 1, NULL, &waitForChild, NULL);

    for (int i = 2; i < FANS + 2; i++) {
        usleep((rand() % 2000 + 1000) * 1000);

        switch (fork()) {
            case -1:
                perror("Blad wydzielenia procesu");
                i--;
                break;
            case 0:
                if (execl("./kibic", "kibic", NULL) == -1) {
                    perror("Blad wywolania procesu");
                }

                break;
            default:
                pthread_create(waitingThreads + i, NULL, &waitForChild, NULL);
        }
    }

    pthread_join(waitingThreads[0], NULL);

    return 0;
}

void* waitForChild() {
    int status;
    int pid = wait(&status);

    if (pid == -1 || status != 0) {
        perror("Blad zakonczenia procesu:");
    } else {
        printf("Proces %d zakonczyl pomyslnie dzialanie.\n", pid);
    }
}

void finishWorking() {
    pthread_join(waitingThreads[0], NULL);
    exit(0);
}