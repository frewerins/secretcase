#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>


volatile sig_atomic_t must_exit = 0;

void handle_sigint(int signal) {
    must_exit = 1;
}

void handle_sigterm(int signal) {
    must_exit = 1;
}

void final(int fd) {
    shutdown(fd, 2);
    close(fd);
}

int main(int argc, char *argv[]) {
    int port_num = strtol(argv[1], NULL, 10);
    char* dir_path = argv[2];
    char* s_addr = "127.0.0.1";

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    const struct sockaddr_in addr = {
            .sin_family = AF_INET,
            .sin_addr = inet_addr(s_addr),
            .sin_port = htons(port_num)
    };

    bind(sock, (const struct sockaddr*)&addr, sizeof(addr));

    listen(sock, SOMAXCONN);

    struct sigaction action_int;
    memset(&action_int, 0, sizeof(sigaction));
    action_int.sa_handler = handle_sigint;
    sigaction(SIGINT, &action_int, NULL);


    struct sigaction action_term;
    memset(&action_term, 0, sizeof(sigaction));
    action_term.sa_handler = handle_sigterm;
    sigaction(SIGTERM, &action_term, NULL);


    while (must_exit != 1) {
        int conn_fd;
        if((conn_fd = accept(sock, (struct sockaddr*)NULL, NULL)) == -1)
            return 1;
        char cur_file[1000];
        int len_dir_path = strlen(dir_path) + 1;
        for (int i = 0; i < len_dir_path; i++)
            cur_file[i] = dir_path[i];
        cur_file[len_dir_path - 1] = '/';


        FILE* file_s = fdopen(conn_fd, "r");
        char inp_file[1000];
        fscanf(file_s, "GET %s HTTP/1.1", inp_file);
        for (int i = len_dir_path; i < len_dir_path + strlen(inp_file); i++) {
            cur_file[i] = inp_file[i - len_dir_path];
        }
        for (int i = len_dir_path + strlen(inp_file); i < 1000; i++)
            cur_file[i] = '\0';


        int cnt = 0;
        while (1) {
            if (cnt == 0 || cnt == 2) {
                char cur = fgetc(file_s);
                if (cur == '\r')
                    cnt++;
                else
                    cnt = 0;
            } else {
                char cur = fgetc(file_s);
                if (cur == '\n')
                    cnt++;
                else {
                    if (cur == '\r')
                        cnt = 1;
                    else
                        cnt = 0;
                }

                if (cnt == 4)
                    break;
            }
        }

        if (access(cur_file, 0) != -1) {
            if (access(cur_file, R_OK) != -1) {
                dprintf(conn_fd, "HTTP/1.1 200 OK\r\n");
                FILE* file = fopen(cur_file, "r");

                struct stat cur_file_st;

                stat(cur_file, &cur_file_st);
                if (dprintf(conn_fd, "Content-Length: %d\r\n\r\n", (int)cur_file_st.st_size) < 0) {
                    return 1;
                }
                char file_content[cur_file_st.st_size + 1];
                fread(file_content, cur_file_st.st_size, 1, file);
                write(conn_fd, file_content, cur_file_st.st_size);
                final(conn_fd);
                continue;

            } else {
                dprintf(conn_fd, "HTTP/1.1 403 Forbidden\r\n");
                final(conn_fd);
                continue;
            }

        } else {
            dprintf(conn_fd, "HTTP/1.1 404 Not Found\r\n");
            final(conn_fd);
            continue;
        }

    }
    close(sock);
    return 0;
}