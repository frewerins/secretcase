#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/signalfd.h>
#include <unistd.h>

volatile sig_atomic_t is_exit = 0;

static void
handler_sigrtmin(int signal, siginfo_t* info_about_signal, void* ucontext)
{
    if (info_about_signal->si_value.sival_int > 0) {
        --info_about_signal->si_value.sival_int;
        sigqueue(
            info_about_signal->si_pid, SIGRTMIN, info_about_signal->si_value);
    } else {
        is_exit = 1;
    }
}

int main()
{
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    sigdelset(&mask, SIGRTMIN);
    printf("%d\n", getpid());
    fflush(stdout);
    sigaction(
        SIGRTMIN,
        &(struct sigaction){.sa_sigaction = handler_sigrtmin,
                            .sa_flags = SA_SIGINFO},
        NULL);
    while (!is_exit) {
        sigsuspend(&mask);
    }
    return 0;
}