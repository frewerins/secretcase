#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int N;
    int k;
    double *arr;
    pthread_mutex_t *mutex;
} common_arg_t;

typedef struct {
    common_arg_t *arg_;
    pthread_t thread;
    int index;
} thread_func_arg_t;

void *thread_func(thread_func_arg_t *arg) {
    int left = arg->index - 1;
    int right = arg->index + 1;
    if (left < 0) {
        left = arg->arg_->k - 1;
    }
    if (right == arg->arg_->k) {
        right = 0;
    }
    for (int i = 0; i < arg->arg_->N; ++i) {
        pthread_mutex_lock(&arg->arg_->mutex[arg->index]);
        arg->arg_->arr[arg->index] += 1;
        pthread_mutex_unlock(&arg->arg_->mutex[arg->index]);

        pthread_mutex_lock(&arg->arg_->mutex[left]);
        arg->arg_->arr[left] += 0.99;
        pthread_mutex_unlock(&arg->arg_->mutex[left]);

        pthread_mutex_lock(&arg->arg_->mutex[right]);
        arg->arg_->arr[right] += 1.01;
        pthread_mutex_unlock(&arg->arg_->mutex[right]);
    }

}
int main(int argc, char *argv[]) {
    common_arg_t *const_arg = (common_arg_t*)malloc(sizeof(common_arg_t));
    const_arg->k = atoi(argv[2]);
    const_arg->N = atoi(argv[1]);

    double arr[const_arg->k];
    memset(arr, 0, sizeof(arr));
    const_arg->arr = arr;

    pthread_mutex_t mutex[const_arg->k];
    for (int i = 0; i < const_arg->k; ++i) {
        pthread_mutex_t new_mutex = PTHREAD_MUTEX_INITIALIZER;
        mutex[i] = new_mutex;
    }
    const_arg->mutex = mutex;
    thread_func_arg_t threads[const_arg->k];

    for (int i = 0; i < const_arg->k; ++i) {
        threads[i].index = i;
        threads[i].arg_ = const_arg;
        pthread_create(&threads[i].thread, NULL, (void* (*)(void*))thread_func, (void*)&threads[i]);
    }
    for (int i = 0; i < const_arg->k; ++i) {
        pthread_join(threads[i].thread, NULL);
    }
    for (int i = 0; i < const_arg->k; ++i) {
        printf("%.10g\n", const_arg->arr[i]);
    }
    free(const_arg);
    return 0;
}
