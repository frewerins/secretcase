#include <signal.h>
#include <stdio.h>
#include <unistd.h>

volatile sig_atomic_t current_value = 0;
volatile sig_atomic_t is_sigterm = 0;

static void handle_1(int signal)
{
    ++current_value;
}
static void handle_2(int signal)
{
    current_value *= -1;
}
static void sigterm_signal(int signal)
{
    is_sigterm = 1;
    _exit(0);
}

int main()
{
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    sigdelset(&mask, SIGUSR1);
    sigdelset(&mask, SIGUSR2);
    sigdelset(&mask, SIGTERM);
    sigaction(
        SIGUSR1,
        &(struct sigaction){.sa_handler = handle_1, .sa_flags = SA_RESTART},
        NULL);
    sigaction(
        SIGUSR2,
        &(struct sigaction){.sa_handler = handle_2, .sa_flags = SA_RESTART},
        NULL);
    sigaction(
        SIGTERM,
        &(struct sigaction){.sa_handler = sigterm_signal,
                            .sa_flags = SA_RESTART},
        NULL);
    printf("%d\n", getpid());
    fflush(stdout);
    scanf("%d", &current_value);
    while (!is_sigterm) {
        sigsuspend(&mask);
        printf("%d\n", current_value);
        fflush(stdout);
    }
    return 0;
}
