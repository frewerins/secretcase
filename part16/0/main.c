#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void *read_and_write(void *arg)
{
    int digit;
    if (scanf("%d", &digit) == 1) {
        pthread_t next_thread;
        pthread_create(&next_thread, NULL, read_and_write, 0);
        pthread_join(next_thread, NULL);
        printf("%d\n", digit);
    }
    return NULL;
}

int main() {
    pthread_t thread;
    pthread_create(&thread, NULL, read_and_write, 0);
    pthread_join(thread, NULL);
    return 0;
}
