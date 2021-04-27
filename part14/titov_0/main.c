#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#define conditional_handle_error(stmt, msg) \
do { if (stmt) { perror(msg " (" #stmt ")"); exit(EXIT_FAILURE); } } while (0)

void write_smth(int fd) {
    int input;
    while (scanf("%d", &input) != EOF) {
        int write_ret = write(fd, &input, sizeof(input));
        if (write_ret == 0) {
            return;
        }
        conditional_handle_error(write_ret != sizeof(input), "writing failed");
        char responseStr[sizeof(int)];
        int bytes_received = 0;
        while (bytes_received < sizeof(int)) {
            int read_ret = read(fd, responseStr + bytes_received, sizeof(responseStr) - bytes_received);
            if (read_ret == 0) {
                return;
            }
            conditional_handle_error(read_ret < 0, "reading failed");
            bytes_received += read_ret;
        }
        int response = *((int*)responseStr);

        printf("%d ", response);
        struct timespec t = {.tv_sec = 0, .tv_nsec = 10000};
        nanosleep(&t, &t);
    }
}

int main(int argc, char* argv[]) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    conditional_handle_error(socket_fd == -1, "can't initialize socket");
    unsigned short port = atoi(argv[2]);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    struct hostent *hosts = gethostbyname(argv[1]);
    conditional_handle_error(!hosts, "can't get host by name");
    memcpy(&addr.sin_addr, hosts->h_addr_list[0], sizeof(addr.sin_addr));

    int connect_ret = connect(socket_fd, (struct sockaddr*)&addr, sizeof(addr));
    conditional_handle_error(connect_ret == -1, "can't connect to unix socket");

    write_smth(socket_fd);
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
    return 0;
}