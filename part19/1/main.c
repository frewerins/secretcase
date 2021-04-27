#include <stdio.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <zconf.h>
#include <stdbool.h>
#include <string.h>

const char* MEMORY_NAME = "/diht82605";

typedef struct {
    sem_t request_ready;  // начальное значение 0
    sem_t response_ready; // начальное значение 0
    char func_name[20];
    double value;
    double result;
} shared_data_t;

int main(int argc, char* argv[]) {
    void *lib = dlopen(argv[1], RTLD_NOW);

    int memory_fd = shm_open(MEMORY_NAME, O_CREAT | O_RDWR, 0644);
    ftruncate(memory_fd, sizeof(shared_data_t));
    shared_data_t* data = mmap(NULL, sizeof(shared_data_t),
                               PROT_READ | PROT_WRITE, MAP_SHARED, memory_fd, 0);
    sem_init(&data->request_ready, 1, 0);
    sem_init(&data->response_ready, 1, 0);
    printf("%s\n", MEMORY_NAME);
    fflush(stdout);
    while (true) {
        sem_wait(&data->request_ready);
        if (strlen(data->func_name) == 0) {
            break;
        }
        double (*function)(double) = dlsym(lib, data->func_name);
        data->result = function(data->value);
        sem_post(&data->response_ready);
    }
    sem_destroy(&data->response_ready);
    sem_destroy(&data->request_ready);
    dlclose(lib);
    shm_unlink(MEMORY_NAME);
    munmap(data, sizeof(shared_data_t));
    close(memory_fd);
    return 0;
}