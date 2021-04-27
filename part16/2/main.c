#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/socket.h>

typedef struct {
    int fd;
    int diff;
} thread_func_arg_t;

static void *thread_func(const thread_func_arg_t *arg) {
    int digit;
    while (read(arg->fd, &digit, sizeof(digit))) {
        digit += arg->diff;
        printf("%d\n", digit);
        if (digit == 0 || digit > 100) {
            break;
        }
        write(arg->fd, &digit, sizeof(digit));
    }
    close(arg->fd);
    shutdown(arg->fd, SHUT_RDWR);
}



int main(int argc, char *argv[]) {
    int n = atoi(argv[1]);
    int fd[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
    write(fd[1], &n, sizeof(n));
    thread_func_arg_t arg_1 = {.fd = fd[0], .diff = -3 };
    thread_func_arg_t arg_2 = {.fd = fd[1], .diff = 5 };
    pthread_t thread_1, thread_2;
    pthread_create(&thread_1, NULL, (void* (*)(void*))thread_func, (void*)&arg_1);
    pthread_create(&thread_2, NULL, (void* (*)(void*))thread_func, (void*)&arg_2);
    pthread_join(thread_1, NULL);
    pthread_join(thread_2, NULL);
    return 0;
}
