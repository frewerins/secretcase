#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

char* create_request(char* request, char* host, int index, uint16_t request_id) {
    char* ptr_of_request = request + index;
    char* ptr_of_host = ptr_of_request + 1;

    int i = 0;
    while (host[i] != '\0') {
        if (host[i] == '.') {
            *ptr_of_request = (uint8_t)((ptr_of_host - ptr_of_request - 1));
            memcpy(ptr_of_request + 1, (host + i) - (size_t)(*ptr_of_request), (size_t)(*ptr_of_request));
            ptr_of_request = ptr_of_host;
        }
        ++i;
        ++ptr_of_host;
    }
    *ptr_of_request = (uint8_t)(ptr_of_host - 1 - ptr_of_request);
    memcpy(ptr_of_request + 1, (host + i) - (size_t)(*ptr_of_request), (size_t)(*ptr_of_request));
    return ptr_of_host;
}

void printing(uint8_t* answer, int size) {
    for (size_t i = 0; i < 4; ++i) {
        printf("%d", answer[size - 4 +i]);
        if (i < 3) {
            printf(".");
        }
    }
    printf("\n");
}



int main() {
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    char host[1024];
    uint16_t request_id = 1;
    while (scanf("%s", host) != EOF) {
        char request[1024];

        memcpy(request, &request_id, sizeof(request_id));
        char beginning[] = {0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x0, 0x00};
        memcpy(request + sizeof(request_id), beginning, sizeof(beginning));
        
        char* ptr_of_host = create_request(request, host, sizeof(request_id) + sizeof(beginning), request_id);

        char end[] = {0x00, 0x00, 0x01, 0x00, 0x01};
        memcpy(ptr_of_host, end, sizeof(end));
        ptr_of_host += sizeof(end);
        size_t size = ptr_of_host - request;

        struct sockaddr_in addr = {
                .sin_family = AF_INET,
                .sin_addr = inet_addr("8.8.8.8"),
                .sin_port = htons(53)
        };
        sendto(socket_fd, request, size, 0, (struct sockaddr*)&addr, sizeof(addr));

        uint8_t reply_from_dns[1024];
        int reply = recv(socket_fd, reply_from_dns, sizeof(reply_from_dns), 0);
        printing(reply_from_dns, reply);
        ++request_id;
    }
    close(socket_fd);
    return 0;
}
