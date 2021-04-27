#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <unistd.h>

const size_t SIZE = 1024;

void send_to(int socket_fd, char* buffer, size_t size) {
    size_t i = 0;
    bool stop = false;
    while (i < size && !stop) {
        int bytes;
        if ((bytes = write(socket_fd, buffer + i, size - i)) == 0) {
            stop = true;
        }
        i += bytes;
    }
}

void read_and_send(char* file, size_t size, int socket_fd, char *buffer) {
    if (size <= 0) {
        return;
    }
    size_t i = 0;
    int fd = open(file, O_RDONLY);
    while (i < size) {
        int bytes = read(fd, buffer, sizeof(buffer));
        send_to(socket_fd, buffer, bytes);
        i += bytes;
    }
    close(fd);
}

int main(int argc, char* argv[])
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    char request[SIZE];

    struct stat st;
    stat(argv[3], &st);
    sprintf(request,
            ("POST %s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/octet-stream\r\n"
             "Content-Length: %d\r\nConnection: close\r\n\r\n"),
            argv[2], argv[1], (int)st.st_size);

    struct addrinfo addr_hints = {.ai_family = AF_INET, .ai_socktype = SOCK_STREAM};
    struct addrinfo* addr_result = NULL;
    getaddrinfo(argv[1], "http", &addr_hints, &addr_result);

    connect(socket_fd, addr_result->ai_addr, addr_result->ai_addrlen);

    send_to(socket_fd, request, strlen(request));

    char buffer[SIZE];
    read_and_send(argv[3], st.st_size, socket_fd, buffer);
    char answer[SIZE + 1];
    memset(answer, 0, sizeof(answer));
    bool stop_first = false;
    bool stop = false;
    int bytes;
    while (!stop_first && !stop) {
        if ((bytes = read(socket_fd, answer, SIZE)) == 0) {
            stop = true;
        } else {
            char *header_end = strstr(answer, "\r\n\r\n");
            if (header_end) {
                stop_first = true;
                header_end += 4;
                printf("%s", header_end);
            } else {
                printf("%s", answer);
            }
            if (bytes < SIZE) {
                stop = true;
            } else {
                memset(answer, 0, sizeof(answer));
            }
        }
    }
    while (!stop) {
        if ((bytes = read(socket_fd, answer, SIZE)) != 0) {
            printf("%s", answer);
            if (bytes < SIZE) {
                stop = true;
            } else {
                memset(answer, 0, sizeof(answer));
            }
        } else {
            stop = true;
        }
    }
    close(socket_fd);
    return 0;
}