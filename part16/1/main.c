#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

typedef struct {
    int count_left;
    int64_t sum;
} thread_func_arg_t;

static void *thread_func(thread_func_arg_t *arg)
{
    pthread_t new_thread;
    thread_func_arg_t *new_arg = (thread_func_arg_t*)malloc(sizeof(thread_func_arg_t));
    new_arg->count_left = arg->count_left - 1;
    new_arg->sum = 0;
    if (arg->count_left > 0) {
        pthread_create(&new_thread, NULL, (void* (*)(void*))thread_func, (void*)new_arg);
    }
    int digit;
    int64_t sum = 0;
    while (scanf("%d", &digit) == 1) {
        sum += digit;
    }
    if (arg->count_left > 0) {
        pthread_join(new_thread, NULL);
    }
    arg->sum = sum + new_arg->sum;
    free(new_arg);
    return NULL;
}

int main(int argc, char *argv[]) {
    thread_func_arg_t arg = {atoi(argv[1]) - 1, 0};
    pthread_t thread;
    pthread_create(&thread, NULL, (void* (*)(void*))thread_func, (void*)&arg);
    pthread_join(thread, NULL);
    printf("%ld\n", arg.sum);
    return 0;
}