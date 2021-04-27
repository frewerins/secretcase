#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char* argv[]) {
    struct addrinfo addr_hints = {.ai_family = AF_INET, .ai_socktype = SOCK_STREAM};
    struct addrinfo* addr_result = NULL;
    getaddrinfo(argv[1], "http", &addr_hints, &addr_result);
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    connect(socket_fd, addr_result->ai_addr, addr_result->ai_addrlen);
    char request[4096];
    snprintf(request, sizeof(request),
            "GET %s HTTP/1.1\nHost: %s\nConnection: close\n\n", argv[2], argv[1]);
    write(socket_fd, request, strnlen(request, sizeof(request)));
    FILE* in = fdopen(socket_fd, "r");
    char buf[65536];
    bool completed = false;
    while (fgets(buf, sizeof(buf), in)) {
        if (strcmp(buf, "\n") == 0 || strcmp(buf, "\r\n") == 0) {
            completed = true;
            continue;
        }
        if (completed) {
            printf("%s", buf);
        }
    }
    fclose(in);
    return 0;
}
