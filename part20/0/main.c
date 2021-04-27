#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>


struct packed_data {
    struct ether_header ethernet_header;
    unsigned char htype[2];
    unsigned char ptype[2];
    unsigned char hlen;
    unsigned char plen;
    unsigned char oper[2];
    unsigned char from_mac_addr[ETH_ALEN];
    unsigned char from_ip_addr[4];
    unsigned char to_mac_addr[ETH_ALEN];
    unsigned char to_ip_addr[4];
} __attribute__((packed));

int main(int argc, char* argv[]) {
    int socket_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    struct ifreq ifr;
    strcpy(ifr.ifr_name, argv[1]);

    ioctl(socket_fd, SIOCGIFINDEX, &ifr);
    struct sockaddr_ll device = {
            .sll_family = AF_PACKET,
            .sll_ifindex = ifr.ifr_ifindex,
            .sll_halen = ETH_ALEN,
            .sll_addr = {0, 0, 0, 0, 0, 0}
    };

    char data_buff[sizeof(struct packed_data)];
    struct packed_data* data = (void*)data_buff;

    ioctl(socket_fd, SIOCGIFADDR, &ifr);
    struct sockaddr_in* socket_addr = (struct sockaddr_in*)&ifr.ifr_addr;
    memcpy(data->from_ip_addr, &(socket_addr->sin_addr.s_addr), sizeof(unsigned long));

    ioctl(socket_fd, SIOCGIFHWADDR, &ifr);
    memcpy(data->ethernet_header.ether_shost, ifr.ifr_hwaddr.sa_data, sizeof(data->ethernet_header.ether_shost));

    char apr_type[2] = {0x08, 0x06};
    memcpy(&data->ethernet_header.ether_type, apr_type, sizeof(apr_type));
    memcpy(data->from_mac_addr, ifr.ifr_hwaddr.sa_data, sizeof(data->from_mac_addr));

    memset(data->ethernet_header.ether_dhost, 0xFF, sizeof(data->ethernet_header.ether_dhost));
    data->oper[0] =  0x00;
    data->oper[1] = 0x01;
    data->ptype[0] = 0x08;
    data->ptype[1] = 0x00;
    data->htype[0] = 0x00;
    data->htype[1] = 0x01;
    data->plen = 0x04;
    data->hlen = 0x06;

    char ip_addr[16];
    while (scanf("%s", ip_addr) != EOF) {
        struct sockaddr_in in_addr;
        inet_aton(ip_addr, &in_addr.sin_addr);
        memcpy(data->to_ip_addr, &in_addr.sin_addr.s_addr, sizeof(unsigned long));

        sendto(socket_fd, data_buff, sizeof(data_buff), 0, (struct sockaddr*)&device, sizeof(device));

        char reply_buff[sizeof(struct packed_data)];
        bool find_ip = false;
        while (!find_ip) {
            recv(socket_fd, reply_buff, sizeof(reply_buff), 0);
            struct packed_data* reply_data = (void*)reply_buff;
            ioctl(socket_fd, SIOCGIFHWADDR, &ifr);
            if (memcmp(reply_data->to_mac_addr, ifr.ifr_hwaddr.sa_data, sizeof(reply_data->to_mac_addr)) == 0) {
                for (int i = 0; i < sizeof(reply_data->from_mac_addr); ++i) {
                    printf("%02x", reply_data->from_mac_addr[i]);
                    if (i < sizeof(reply_data->from_mac_addr) - 1) {
                        printf(":");
                    }
                }
                printf("\n");
                find_ip = true;
            }
        }
    }
    close(socket_fd);

    return 0;
}