#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

typedef struct Item {
    struct Item *next;
    int64_t      value;
} item_t;

typedef struct {
    _Atomic (item_t*) head;
    _Atomic (item_t*) tail;
    int N;
    int k;
} const_args_t;

typedef struct {
    const_args_t* cntx;
    int number;
} thread_func_arg_t;

static void *insert(thread_func_arg_t *arg) {
    for (int i = 0; i < arg->cntx->k; ++i) {
        int64_t value = arg->number * arg->cntx->k + i;
        item_t *new_node = malloc(sizeof(item_t));
        new_node->value = value;
        new_node->next = NULL;
        item_t *test = NULL;
        if (atomic_compare_exchange_strong(&arg->cntx->tail, &test, new_node)) {
            atomic_store(&arg->cntx->head, new_node);
        } else {
            item_t *tail = atomic_exchange(&arg->cntx->tail, new_node);
            tail->next = new_node;
        }
    }
}


int main(int argc, char *argv[]) {
    const_args_t const_args = { .k = atoi(argv[2]), .N = atoi(argv[1]), .head = NULL, .tail = NULL};
    thread_func_arg_t args[const_args.N];
    pthread_t threads[const_args.N];
    for (int i = 0; i < const_args.N; ++i) {
        args[i].cntx = &const_args;
        args[i].number = i;
        pthread_create(&threads[i], NULL, (void*(*)(void*))insert, (void*)&args[i]);
    }
    for (int i = 0; i < const_args.N; ++i) {
        pthread_join(threads[i], NULL);
    }
    item_t *node = args->cntx->head;
    item_t *prev;
    while (node != NULL) {
        printf("%ld\n", node->value);
        prev = node;
        node = node->next;
        free(prev);
    }
    return 0;
}
