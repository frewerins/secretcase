#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <wait.h>

int fd_for_signals[2];
const int MAX_LENGTH = 1500;

void handle(int signal) {
    int written = write(fd_for_signals[1], "X", 1);
    close(fd_for_signals[1]);
}

void receive_strings(FILE* file) {
    int check = 0;
    while (check < 4) {
        char symb = fgetc(file);
        if ((check % 2) == 0) {
            symb == '\r' ? ++check : (check = 0);
        } else {
            symb == '\n' ? ++check : (check = (symb == '\r' ? 1 : 0));
        }
    }
}

int processing(int socket_fd, char *direction) {
    int connect_fd = accept(socket_fd, (struct sockaddr *) NULL, NULL);
    if (connect_fd == -1) {
        return 1;
    }
    char all_file_name[MAX_LENGTH];

    for (int i = 0; i < strlen(direction); ++i) {
        all_file_name[i] = direction[i];
    }
    all_file_name[strlen(direction)] = '/';


    FILE *connect_file = fdopen(connect_fd, "r");
    char file_name[MAX_LENGTH];
    fscanf(connect_file, "GET %s HTTP/1.1", file_name);
    for (int i = strlen(direction) + 1; i < strlen(direction) + 1 + strlen(file_name); ++i) {
        all_file_name[i] = file_name[i - strlen(direction) - 1];
    }
    for (int i = strlen(direction) + 1 + strlen(file_name); i < MAX_LENGTH; ++i) {
        all_file_name[i] = '\0';
    }

    receive_strings(connect_file);

    if (access(all_file_name, 0) == -1) {
        dprintf(connect_fd, "HTTP/1.1 404 Not Found\r\n");
    } else if (access(all_file_name, R_OK) == -1) {
        dprintf(connect_fd, "HTTP/1.1 403 Forbidden\r\n");
    } else {
        dprintf(connect_fd, "HTTP/1.1 200 OK\r\n");
        FILE *file = fopen(all_file_name, "r");
        struct stat all_file_name_st;
        stat(all_file_name, &all_file_name_st);

        if (access(all_file_name, 1) == -1) {
            if (dprintf(connect_fd, "Content-Length: %d\r\n\r\n", (int) all_file_name_st.st_size) >= 0) {
                char file_content[all_file_name_st.st_size + 1];
                fread(file_content, all_file_name_st.st_size, 1, file);
                write(connect_fd, file_content, all_file_name_st.st_size);
            }
        } else {
            pid_t pid;
            if ((pid = fork()) == 0) {
                dup2(connect_fd, 1);
                close(connect_fd);
                execlp(all_file_name,NULL );
            }
            waitpid(pid, 0, 0);
        }
    }
    shutdown(connect_fd, SHUT_RDWR);
    close(connect_fd);
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

    struct sigaction sigint = { .sa_handler = handle }, sigterm = { .sa_handler = handle };
    sigaction(SIGINT, &sigint, NULL);
    sigaction(SIGTERM, &sigterm, NULL);

    int fds[] = {stop_fd, socket_fd, -1};
    for (int *fd = fds; *fd != -1; ++fd) {
        struct epoll_event event = {
                .events = EPOLLIN | EPOLLERR | EPOLLHUP,
                .data = {.fd = *fd}
        };
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, *fd, &event);
    }

    while (true) {
        struct epoll_event event;
        int epoll_ret = epoll_wait(epoll_fd, &event, 1, 1000);
        if (epoll_ret <= 0) {
            continue;
        }
        if (event.data.fd == stop_fd) {
            break;
        }
        if (processing(socket_fd, argv[2])) {
            return 0;
        }
    }

    close(epoll_fd);
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
    return 0;
}