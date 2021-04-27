#define _GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int n = argc - 1;
    int pid[n];
    int pair[2][2];
    pipe2(pair[0], O_NONBLOCK);
    pipe2(pair[1], O_NONBLOCK);
    int i = 0;
    while (i < n) {
        if ((pid[i] = fork()) == 0) {
            if (i % 2 == 0) {
                if (i < n - 1) {
                    dup2(pair[0][1], 1);
                }
                if (i > 0) {
                    dup2(pair[1][0], 0);
                }
                execlp(argv[i + 1], argv[i + 1], NULL);
            } else {
                if (i < n - 1) {
                    dup2(pair[1][1], 1);
                }
                dup2(pair[0][0], 0);
                execlp(argv[i + 1], argv[i + 1], NULL);
            }
        } else {
            waitpid(pid[i], 0, 0);
            ++i;
        }
    }
    close(pair[0][0]);
    close(pair[0][1]);
    close(pair[1][0]);
    close(pair[1][1]);
    for (int i = 0; i < n; ++i) {
        waitpid(pid[i], 0, 0);
    }
    return 0;
};
