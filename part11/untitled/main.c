#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

volatile sig_atomic_t running = 1;
volatile sig_atomic_t counter = 0;

static void handler(int sig)
{
    if (sig == SIGINT) {
        counter++;
    } else if (sig == SIGTERM) {
        running = 0;
    }
}

int main()
{
    struct sigaction action;
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
// sigaddset(&action.sa_mask, SIGINT);
// sigaddset(&action.sa_mask, SIGTERM);

    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);

    pid_t pid = getpid();
    printf("%d\n", pid);
    fflush(stdout);

    while (running)
        sleep(1);

    printf("%d", counter);
    return 0;
}