#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/signalfd.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    FILE* files[argc - 1];
    for (int i = 0; i < argc - 1; ++i) {
        files[i] = fopen(argv[i + 1], "r");
    }
    sigset_t mask;
    sigset_t mask_with_catch_signals;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    sigemptyset(&mask_with_catch_signals);
    for (int i = 0; i < argc; ++i) {
        sigaddset(&mask_with_catch_signals, SIGRTMIN + i);
    }
    int signals = signalfd(-1, &mask_with_catch_signals, 0);
    struct signalfd_siginfo signal;
    while (1) {
        read(signals, &signal, sizeof(signal));
        int signal_index = signal.ssi_signo - SIGRTMIN;
        if (signal_index > 0) {
            char str[4096];
            memset(str, 0, sizeof(str));
            fgets(str, sizeof(str), files[signal_index - 1]);
            fputs(str, stdout);
            fflush(stdout);
        } else {
            break;
        }
    }
    close(signals);
    for (int i = 0; i < argc - 1; ++i) {
        fclose(files[i]);
    }
    return 0;
}