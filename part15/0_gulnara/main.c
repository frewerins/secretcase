#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <zconf.h>
#include <sys/epoll.h>

typedef struct {
    int fd;
    size_t count;
    bool done;
} data_t;

void
reading_data(data_t *data, int epoll_fd, int *files_not_done)
{
    char buffer[4096];
    int count = read(data->fd, buffer, sizeof(buffer));
    if (count == 0) {
        data->done = true;
        close(data->fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, data->fd, NULL);
        *files_not_done -= 1;
    } else if (count > 0) {
        data->count += count;
    }
}

data_t*
create_event_data(int epoll_fd, int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);

    data_t *data = calloc(1, sizeof(data));
    data->fd = fd;

    struct epoll_event event = {.events = EPOLLIN, .data.ptr = data};
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    return data;
}

extern size_t
read_data_and_count(size_t N, int in[N])
{
    data_t *entr[N];
    int epoll_fd = epoll_create1(0);
    for (int i = 0; i < N; ++i) {
        entr[i] = create_event_data(epoll_fd, in[i]);
    }
    int files_not_done = N;
    while (files_not_done > 0) {
        struct epoll_event current_events[N];
        int count_events = epoll_wait(epoll_fd, current_events, N, -1);
        for (int i = 0; i < count_events; ++i) {
            if (current_events[i].events & EPOLLIN) {
                reading_data(current_events[i].data.ptr, epoll_fd, &files_not_done);
            }
        }
    }
    close(epoll_fd);

    size_t result = 0;
    for (int i = 0; i < N; ++i) {
        result += entr[i]->count;
        free(entr[i]);
    }
    return result;
}