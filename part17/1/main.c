#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct {
    int64_t left;
    int64_t right;
    int stop;
    bool printed;
    bool found;
    int n;
    int64_t digit;
    pthread_mutex_t *mutex;
    pthread_cond_t *condvar_find;
    pthread_cond_t *condvar_print;
} thread_func_arg_t;

int is_prime(int64_t n) {
    if (n == 1) {
        return 0;
    }
    if (n == 2 || n == 3) {
        return 1;
    }
    if (n % 2 == 0) {
        return 0;
    }
    int i = 3;
    while (i * i <= n) {
        if (n % i == 0) {
            return 0;
        }
        i += 2;
    }
    return 1;
}

static void *thread_func(thread_func_arg_t *arg) {
    int64_t i = arg->left;
    int count = 0;
    while (i <= arg->right && count < arg->n) {
        if (is_prime(i)) {
            pthread_mutex_lock(arg->mutex);
            while (!arg->printed) {
                pthread_cond_wait(arg->condvar_print, arg->mutex);
            }
            arg->printed = false;
            arg->digit = i;
            arg->found = true;
            pthread_cond_signal(arg->condvar_find);
            ++count;
            pthread_mutex_unlock(arg->mutex);
        }
        ++i;
    }
    pthread_mutex_lock(arg->mutex);
    pthread_cond_wait(arg->condvar_print, arg->mutex);
    arg->stop = 1;
    arg->found = true;
    pthread_cond_signal(arg->condvar_find);
    pthread_mutex_unlock(arg->mutex);
    return NULL;
}

int main(int argc, char *argv[]) {
    int64_t A = strtol(argv[1], NULL, 10);
    int64_t B = strtol(argv[2], NULL, 10);
    int N = strtol(argv[3], NULL, 10);

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t condvar_find = PTHREAD_COND_INITIALIZER;
    pthread_cond_t condvar_print = PTHREAD_COND_INITIALIZER;

    thread_func_arg_t arg = { .left = A, .right = B, .n = N, .mutex = &mutex, .condvar_find = &condvar_find,
                              .condvar_print = &condvar_print, .stop = 0, .printed = true, .found = false };
    pthread_t thread;
    pthread_mutex_lock(arg.mutex);
    pthread_create(&thread, NULL, (void* (*)(void*))thread_func, (void*)&arg);
    while (!arg.stop) {
        while (!arg.found) {
            pthread_cond_wait(arg.condvar_find, arg.mutex);
        }
        arg.found = false;
        if (!arg.stop) {
            int64_t number = arg.digit;
            pthread_mutex_unlock(arg.mutex);
            printf("%ld\n", number);
            pthread_mutex_lock(arg.mutex);
            arg.printed = true;
            pthread_cond_signal(arg.condvar_print);
        }
    }
    pthread_mutex_unlock(arg.mutex);
    pthread_join(thread, NULL);//
    return 0;
}
