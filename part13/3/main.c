#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int n = argc - 1;
    int pid[n];
    int pair[n - 1][2];
    for (int i = 0; i < n - 1; ++i) {
        pipe(pair[i]);
    }
    int i = 0;
    while (i < n) {
        if ((pid[i] == fork()) == 0) {
            if (i > 0) {
                dup2(pair[i - 1][0],0);
            }
            if (i < n - 1) {
                dup2(pair[i][1], 1);
            }
            for (int i = 0; i < n - 1; ++i) {
                close(pair[i][0]);
                close(pair[i][1]);
            }
            execlp(argv[i + 1], argv[i + 1], NULL);
        }
        ++i;
    }
    for (int i = 0; i < n - 1; ++i) {
        close(pair[i][0]);
        close(pair[i][1]);
    }
    for ( i = 0; i < n; ++i) {
        waitpid(pid[i], 0, 0);
    }
    return 0;
};