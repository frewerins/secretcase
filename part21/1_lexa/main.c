#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

int send_data(int sockfd, char* buffer, size_t size)
{
    int sent = 0;
    do {
        int bytes = write(sockfd, buffer + sent, size - sent);
        if (bytes < 0) {
            printf("ERROR writing message to socket");
        }
        if (bytes == 0) {
            break;
        }
        sent += bytes;
    } while (sent < size);
    return sent;
}

int main(int argc, char* argv[])
{
    char* host = argv[1];
    char* script = argv[2];
    char* file_name = argv[3];
    char request_body[1024];
    char request_data_buffer[1024];
    char response[1025];

    struct stat st;
    stat(file_name, &st);
    // int size = st.st_size;
    // http://httpbin.org/#/HTTP_Methods/post_post

    sprintf(request_body,
            ("POST %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Content-Type: application/octet-stream\r\n"
             "Content-Length: %d\r\n"
             "Connection: close\r\n\r\n"),
            script, host, (int)st.st_size);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        printf("ERROR opening socket");

    struct hostent* server;
    server = gethostbyname(host);
    if (server == NULL)
        printf("ERROR: no such host");

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("ERROR during connection");
    }

    send_data(sockfd, request_body, strlen(request_body));

    ssize_t file_bytes_sent = 0;
    if (st.st_size > 0) {
        int source = open(file_name, O_RDONLY);
        if (source == -1) {
            printf("ERROR opening file");
        }

        while (file_bytes_sent < st.st_size) {
            ssize_t read_bytes = read(source, request_data_buffer, sizeof(request_data_buffer));
            if (read_bytes < 0) {
                printf("ERROR reading from file");
            }
            send_data(sockfd, request_data_buffer, read_bytes);
            file_bytes_sent += read_bytes;
        }

        close(source);
    }

    memset(response, 0, sizeof(response));
    int total = sizeof(response) - 1;
    int first_read = 1;
    do {
        int bytes = read(sockfd, response, total);
        if (bytes < 0) {
            printf("ERROR reading response from server");
        }
        if (bytes == 0) {
            break;
        }
        if (first_read) {
            char* end_of_header = strstr(response, "\r\n\r\n");
            if (end_of_header) {
                end_of_header += 4;
                printf("%s", end_of_header);
                first_read = 0;
            } else {
                printf("%s", response);
            }
        } else {
            printf("%s", response);
        }
        if (bytes < total) {
            break;
        }
        memset(response, 0, sizeof(response));
    } while (1);

    close(sockfd);

    return 0;
}