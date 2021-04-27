#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int port = atoi(argv[2]);

    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        //client
        int port = strtol(argv[2], NULL, 10);
        int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);

        struct hostent *hosts = gethostbyname(argv[1]);
        memcpy(&addr.sin_addr, hosts->h_addr_list[0], sizeof(addr.sin_addr));

        int connect_ret = connect(socket_fd, (struct sockaddr *) &addr, sizeof(addr));
        if (connect_ret != -1) {
            int number_send, number_receive;
            while (scanf("%d", &number_send) > 0) {
                write(socket_fd, &number_send, sizeof(number_send));
                read(socket_fd, &number_receive, sizeof(number_receive));
                printf("%d\n", number_receive);
                fflush(stdout);
            }
            shutdown(socket_fd, SHUT_RDWR);
            close(socket_fd);
        }
    }
    if ((pid_2 = fork()) == 0) {
        //server
        int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(port)};
        int bind_ret = bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr));
        int listen_ret = listen(socket_fd, 2);
        struct sockaddr_in peer_addr = {0};
        socklen_t peer_addr_size = sizeof(struct sockaddr_in);
        int connection_fd = accept(socket_fd, (struct sockaddr*)&peer_addr, &peer_addr_size);
        //connection!!
        int number;
        while ((read(connection_fd, &number, sizeof(number))) > 0) {
            write(connection_fd, &number, sizeof(number));
        }
        shutdown(connection_fd, SHUT_RDWR);
        close(connection_fd);

        shutdown(socket_fd, SHUT_RDWR);
        close(socket_fd);
        return 0;
    }
    waitpid(pid_1, 0, 0);
    waitpid(pid_2, 0, 0);
    return 0;
}
