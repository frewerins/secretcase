#include <signal.h>
#include <stdio.h>
#include <unistd.h>

volatile sig_atomic_t count = 0;
volatile sig_atomic_t is_sigterm = 0;

static void sum(int signal)
{
    ++count;
}
static void sigterm_signal(int signal)
{
    is_sigterm = 1;
}

int main()
{
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    sigdelset(&mask, SIGTERM);
    sigdelset(&mask, SIGINT);
    sigaction(
        SIGINT,
        &(struct sigaction){.sa_handler = sum, .sa_flags = SA_RESTART},
        NULL);
    sigaction(
        SIGTERM,
        &(struct sigaction){.sa_handler = sigterm_signal,
                            .sa_flags = SA_RESTART},
        NULL);
    printf("%d\n", getpid());
    fflush(stdout);
    while (!is_sigterm) {
        sigsuspend(&mask);
    }
    printf("%d\n", count);
    return 0;
}