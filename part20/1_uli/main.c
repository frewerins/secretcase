#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const int PORT_DNS = 53;
const char BEGIN_FROMING[] = {0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x0, 0x00};
const char END_FORMING[] = {0x00, 0x00, 0x01, 0x00, 0x01};

size_t forming_request(char* request_to_dns, char* host_name, uint16_t request_id) {
    //forming the beginning of a request for dns
    memcpy(request_to_dns, &request_id, sizeof(request_id));
    memcpy(request_to_dns + sizeof(request_id), BEGIN_FROMING, sizeof(BEGIN_FROMING));

    char* ptr_number_of_bytes = request_to_dns + sizeof(request_id) + sizeof(BEGIN_FROMING);
    char* host_name_ptr = ptr_number_of_bytes + 1;

    //forming host name
    char* ptr;
    for (ptr = host_name; *ptr != '\0'; ++ ptr) {
        if (*ptr == '.') {
            *ptr_number_of_bytes = (uint8_t)(host_name_ptr - ptr_number_of_bytes - 1);
            memcpy(ptr_number_of_bytes + 1, ptr - (size_t)(*ptr_number_of_bytes), (size_t)(*ptr_number_of_bytes));
            ptr_number_of_bytes = host_name_ptr;
        }
        host_name_ptr++;
    }
    *ptr_number_of_bytes = (uint8_t)(host_name_ptr - ptr_number_of_bytes - 1);
    memcpy(ptr_number_of_bytes + 1, ptr - (size_t)(*ptr_number_of_bytes), (size_t)(*ptr_number_of_bytes));
    ptr_number_of_bytes = host_name_ptr;

    //forming the end of request
    memcpy(host_name_ptr, END_FORMING, sizeof(END_FORMING));
    host_name_ptr += sizeof(END_FORMING);
    return host_name_ptr - request_to_dns;
}

int main()
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket");
        abort();
    }
    struct sockaddr_in addr = {
            .sin_family = AF_INET,
            .sin_port = htons(PORT_DNS),
            .sin_addr = inet_addr("8.8.8.8")
    };
    uint16_t request_id = 1;
    char host_name[4096];

    while (scanf("%s", host_name) != EOF) {
        char request_to_dns[4096];

        size_t size = forming_request(request_to_dns, host_name, request_id);

        if (sendto(sockfd, request_to_dns, size, 0, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
            perror("sendto");
            abort();
        }

        uint8_t answer_dns[4096];
        int answ;
        if ((answ = recv(sockfd, answer_dns, sizeof(answer_dns), 0)) == -1) {
            perror("recv");
            abort();
        }

        for (size_t i = answ - 4; i < answ; ++i) {
            printf("%d", answer_dns[i]);
            if (i + 1 != answ) {
                printf(".");
            }
        }
        printf("\n");

        request_id++;
    }
    if (close(sockfd) != 0) {
        perror("close");
        abort();
    }
    return 0;
}
