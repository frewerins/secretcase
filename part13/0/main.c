#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    char *cmd = argv[1];
    char *in_name = argv[2];
    int pair[2];
    pipe(pair);
    pid_t pid = fork();
    if (pid == 0) {
        int in = open(in_name, O_RDONLY);
        dup2(in, 0);
        close(in);
        dup2(pair[1], 1);
        close(pair[1]);
        execlp(cmd, cmd, NULL);
    } else {
        close(pair[1]);
        uint32_t count = 0;
        uint32_t cur_count;
        char buffer[4096];
        while ((cur_count = read(pair[0], buffer, sizeof(buffer))) > 0) {
            count += cur_count;
        }
        waitpid(pid, 0, 0);
        printf("%u\n", count);
    }
    return 0;
}
