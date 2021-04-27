#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <zconf.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>

int fd_for_signals[2];

void handle(int signal) {
    int written = write(fd_for_signals[1], "X", 1);
    close(fd_for_signals[1]);
}

void
reading_data(struct epoll_event *event)
{
    char buffer[4096];
    int count;
    while ((count = read(event->data.fd, buffer, sizeof(buffer))) > 0) {
        for (int i = 0; i < count; ++i)  {
            buffer[i] = toupper(buffer[i]);
        }
        write(event->data.fd, buffer, count);
    }
    if (count != -1) {
        close(event->data.fd);
    }
}

int processing(int epoll_fd, int socket_fd, struct epoll_event* event) {
    if (event->data.fd == socket_fd) {
        struct sockaddr new_addr;
        socklen_t len = sizeof(new_addr);
        int new_fd = accept(socket_fd, &new_addr, &len);
        fcntl(new_fd, F_SETFL, fcntl(new_fd, F_GETFL) | O_NONBLOCK);

        struct epoll_event new_event = {.data.fd = new_fd, .events = EPOLLIN | EPOLLET};
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_fd, &new_event);
    } else {
        reading_data(event);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    pipe(fd_for_signals);
    int stop_fd = fd_for_signals[0];
    int epoll_fd = epoll_create1(0);

    int port = strtol(argv[1], NULL, 10);
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    struct hostent *hosts = gethostbyname("localhost");
    memcpy(&addr.sin_addr, hosts->h_addr_list[0], sizeof(addr.sin_addr));

    bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(socket_fd, SOMAXCONN);

    struct sigaction sigterm = { .sa_handler = handle };
    sigaction(SIGTERM, &sigterm, NULL);

    struct epoll_event event_stop_fd = {
            .events = EPOLLIN | EPOLLERR | EPOLLHUP,
            .data = {.fd = stop_fd}
    };
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, stop_fd, &event_stop_fd);


    struct epoll_event event_socket_fd = {
            .events = EPOLLET | EPOLLIN,
            .data = {.fd = socket_fd}
    };
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event_socket_fd);

    fcntl(socket_fd, F_SETFL, fcntl(socket_fd, F_GETFL) | O_NONBLOCK);

    while (true) {
        struct epoll_event event;
        int epoll_ret = epoll_wait(epoll_fd, &event, 1, 1000);
        if (epoll_ret <= 0) {
            continue;
        }
        if (event.data.fd == stop_fd) {
            break;
        }
        if (processing(epoll_fd, socket_fd, &event)) {
            return 0;
        }
    }

    close(epoll_fd);
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
    return 0;
}