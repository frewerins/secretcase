#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char *argv[]) {
    short port = strtol(argv[1], NULL, 10);
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock != -1) {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;

        struct hostent *hosts = gethostbyname("localhost");
        memcpy(&addr.sin_addr, hosts->h_addr_list[0], sizeof(addr.sin_addr));
        addr.sin_port = htons(port);

        int number_send, number_recv;
        while (scanf("%d", &number_send) > 0) {
            sendto(sock, &number_send, sizeof(number_send), 0, (const struct sockaddr*) &addr, sizeof(addr));
            recvfrom(sock, &number_recv, sizeof(number_recv), 0, NULL, NULL);
            printf("%d\n", number_recv);
        }
        close(sock);
    }
    return 0;
};
