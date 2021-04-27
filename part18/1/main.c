#include <sys/mman.h>
#include <string.h>
#include <semaphore.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <wait.h>

typedef double (*function_t)(double);

double* pmap_process(function_t func, const double *in, size_t count) {
    double* numbers = mmap(NULL, sizeof(double) * count,
                           PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    size_t count_of_procs = get_nprocs();
    sem_t* semaphore = (void*)(numbers + count);
    sem_init(semaphore, 1, 0);
    size_t start_of_block = 0;
    size_t len_of_block = count / count_of_procs;
    size_t i = 0;
    pid_t pid[count_of_procs];
    while (i < count_of_procs) {
        if ((pid[i] = fork()) == 0) {
            if (i == count_of_procs - 1) {
                len_of_block = count - start_of_block;
            }
            for (size_t j = start_of_block; j < start_of_block + len_of_block; ++j) {
                numbers[j] = func(in[j]);
            }
            sem_post(semaphore);
            _exit(0);
        }
        start_of_block += len_of_block;
        ++i;
    }
    for (size_t i = 0; i < count_of_procs; ++i) {
        sem_wait(semaphore);
    }
    for (size_t i = 0; i < count_of_procs; ++i) {
        waitpid(pid[i], 0, 0);
    }
    sem_destroy(semaphore);
    return numbers;

}

void pmap_free(double *ptr, size_t count) {
    munmap(ptr, sizeof(double) * count + sizeof(sem_t));
}