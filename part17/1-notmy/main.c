#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t condvar;

    bool is_printing; // 1 -- printing, 0 -- picking prime number
} control_t;

void control_init(control_t* control, bool is_printing) {
    pthread_mutex_init(&control->mutex, NULL);
    pthread_cond_init(&control->condvar, NULL);
    control->is_printing = is_printing;
}

void control_release(control_t* control) {
    pthread_mutex_unlock(&control->mutex);
    pthread_cond_signal(&control->condvar);
}

void control_get(control_t* control, bool is_printing) {
    pthread_mutex_lock(&control->mutex);
    while (control->is_printing == is_printing) {
        pthread_cond_wait(&control->condvar, &control->mutex);
    }
    control->is_printing = is_printing;
}

void control_destroy(control_t* control) {
    pthread_mutex_destroy(&control->mutex);
    pthread_cond_destroy(&control->condvar);
}

typedef struct {
    int64_t A;
    int64_t B;
    int32_t N_left;
    int64_t prime;
    control_t* control;
} result_t;

static void* generate_prime(void* arg) {
    result_t* result = arg;
    if (result->N_left == 0) {
        return NULL;
    }
    for (uint64_t i = result->A; i <= result->B && result->N_left; ++i) {
        bool is_prime = true;
        for (uint64_t j = 2; j * j <= i; ++j) {
            if (i % j == 0) {
                is_prime = false;
                break;
            }
        }
        if (is_prime) {
            control_get(result->control, false);
            result->prime = i;
            --result->N_left;
            control_release(result->control);
        }
    }
}


int main(int argc, char* argv[]) {
    int64_t A;
    int64_t B;
    int32_t N;
    sscanf(argv[1], "%ld", &A);
    sscanf(argv[2], "%ld", &B);
    sscanf(argv[3], "%d", &N);

    control_t control;
    control_init(&control, true);
    result_t result = {
            .A = A,
            .B = B,
            .N_left = N,
            .control = &control
    };
    pthread_t prime_generator;
    pthread_create(&prime_generator, NULL, (void*(*)(void*))generate_prime, &result);
    for (uint32_t i = 0; i < N; ++i) {
        control_get(&control, true);
        uint64_t value = result.prime;
        control_release(&control);
        printf("%ld\n", value);
        fflush(stdout);
    }
    pthread_join(prime_generator, NULL);
    control_destroy(&control);
    return 0;
}