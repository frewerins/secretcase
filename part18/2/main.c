#include <sys/mman.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>
#include <wait.h>
#include <stdbool.h>
#include <stdio.h>

const int COUNT_OF_SMOKERS = 3;
const char* TO_SMOKE = "TPM";
volatile sig_atomic_t stop = 0;


void handle(int signal) {
    stop = 1;
}

int main() {
    sem_t* semaphore = mmap(NULL, sizeof(int) * (COUNT_OF_SMOKERS + 1),
            PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    for (int i = 0; i < COUNT_OF_SMOKERS + 1; ++i) {
        sem_init(&semaphore[i], 1, 0);
    }
    pid_t pid[COUNT_OF_SMOKERS];
    for (int i = 0; i < COUNT_OF_SMOKERS; ++i) {
        if ((pid[i] = fork()) == 0) {
            struct sigaction sigterm = { .sa_handler = handle };
            sigaction(SIGTERM, &sigterm, NULL);
            while (true) {
                sem_wait(&semaphore[i]);
                if (stop) {
                    _exit(0);
                }
                printf("%c", TO_SMOKE[i]);
                fflush(stdout);
                sem_post(&semaphore[COUNT_OF_SMOKERS]);
            }
        }
    }
    char word;
    while ((scanf("%c", &word)) != EOF) {
        if (word == 't') {
            sem_post(&semaphore[0]);
        } else if (word == 'p') {
            sem_post(&semaphore[1]);
        } else if (word == 'm') {
            sem_post(&semaphore[2]);
        }
        if (word == 't' || word == 'p' || word == 'm') {
            sem_wait(&semaphore[COUNT_OF_SMOKERS]);
        }
    }
    for (int i = 0; i < COUNT_OF_SMOKERS; ++i) {
        kill(pid[i], SIGTERM);
    }
    for (int i = 0; i < COUNT_OF_SMOKERS; ++i) {
        sem_post(&semaphore[i]);
    }
    for (int i = 0; i < COUNT_OF_SMOKERS; ++i) {
        waitpid(pid[i], 0, 0);
    }
    for (int i = 0; i < COUNT_OF_SMOKERS + 1; ++i) {
        sem_destroy(&semaphore[i]);
    }
    munmap(semaphore, sizeof(int) * (COUNT_OF_SMOKERS + 1));
    return 0;
}
