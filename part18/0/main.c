#include <stdio.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>

int main() {
    char name_of_semafore[4096];
    char memory_block[4096];
    size_t N;
    scanf("%s %s %lu", name_of_semafore, memory_block, &N);

    sem_t* semaphore = sem_open(name_of_semafore, 0);
    sem_wait(semaphore);
    int fd = shm_open(memory_block, O_RDWR, 0);
    int* numbers = mmap(NULL, sizeof(int) * N,
            PROT_READ, MAP_SHARED, fd, 0);
    sem_post(semaphore);
    for (size_t i = 0; i < N; ++i) {
        printf("%d ", numbers[i]);
    }
    shm_unlink(memory_block);
    munmap(numbers, sizeof(int) * N);
    sem_close(semaphore);
}