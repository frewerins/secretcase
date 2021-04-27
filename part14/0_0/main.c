#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int port = strtol(argv[2], NULL, 10);
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    struct hostent *hosts = gethostbyname(argv[1]);
    memcpy(&addr.sin_addr, hosts->h_addr_list[0], sizeof(addr.sin_addr));

    int connect_ret = connect(socket_fd, (struct sockaddr*)&addr, sizeof(addr));
    if (connect_ret != -1) {
        char number_send[4];
        while (scanf("%d", (int*)number_send) != EOF) {
            int bytes = 0;
            char *start = number_send;
            while (bytes < sizeof(int)) {
                int added_bytes;
                if ((added_bytes = write(socket_fd, start, sizeof(int) - bytes)) <= 0) {
                    return 0;
                }
                bytes += added_bytes;
                start += added_bytes;
            }
            int number_receive;
            if (read(socket_fd, &number_receive, sizeof(int)) <= 0) {
                return 0;
            }
           printf("%d\n", number_receive);
        }
        shutdown(socket_fd, SHUT_RDWR);
        close(socket_fd);
    }
    return 0;
};