#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    char *cmd1 = argv[1];
    char *cmd2 = argv[2];
    int pair[2];
    pipe(pair);
    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        dup2(pair[1], 1);
        close(pair[0]);
        close(pair[1]);
        execlp(cmd1, cmd1, NULL);
    }
    if ((pid_2 = fork()) == 0) {
        dup2(pair[0], 0);
        close(pair[0]);
        close(pair[1]);
        execlp(cmd2, cmd2, NULL);
    }
    close(pair[0]);
    close(pair[1]);
    waitpid(pid_1, 0, 0);
    waitpid(pid_2, 0, 0);
    return 0;
}
