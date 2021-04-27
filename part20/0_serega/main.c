#include <arpa/inet.h>
#include <inttypes.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct framed_packet {
    // Ethernet Frame Header
    unsigned char destination_mac[ETH_ALEN];
    unsigned char source_mac[ETH_ALEN];
    unsigned char ethertype[2];

    // ARP Packet
    unsigned char htype[2];
    unsigned char ptype[2];
    unsigned char hlen;
    unsigned char plen;
    unsigned char oper[2];
    unsigned char sha[ETH_ALEN];
    unsigned char spa[4];
    unsigned char tha[ETH_ALEN];
    unsigned char tpa[4];
} __attribute__((packed));

const unsigned char ARP_ETHERTYPE[2] = {0x08, 0x06};
const unsigned char ETHERNET_HTYPE[2] = {0x00, 0x01};
const unsigned char IPV4_PTYPE[2] = {0x08, 0x00};
const unsigned char ETHERNET_HLEN = 0x06;
const unsigned char IPV4_PLEN = 0x04;
const unsigned char OP_REQUEST[2] = {0x00, 0x01};
const unsigned char OP_REPLY[2] = {0x00, 0x02};

const size_t MAX_IP_SIZE = 16;

int main(int argc, char* argv[]) {
    // Create socket
    int socket_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (socket_fd == -1) perror("socket");

    // Create structure for making requests to our network device
    struct ifreq interface_request;
    strcpy(interface_request.ifr_name, argv[1]);

    // Get the interface index of the interface
    int return_code = ioctl(socket_fd, SIOCGIFINDEX, &interface_request);
    if (return_code != 0) perror("ioctl");

    // Create and init socket address
    struct sockaddr_ll socket_address = {
            .sll_family = AF_PACKET,
            .sll_protocol = 0,
            .sll_ifindex = interface_request.ifr_ifindex,
            .sll_hatype = 0,
            .sll_pkttype = 0,
            .sll_halen = ETH_ALEN
    };

    char packet_buffer[sizeof(struct framed_packet)];
    struct framed_packet* packet = (void*)packet_buffer;

    // Fill Ethernet Frame Header =============================================
    memset(packet->destination_mac, 0xFF, sizeof(packet->destination_mac));

    // Get the hardware address of a device
    return_code = ioctl(socket_fd, SIOCGIFHWADDR, &interface_request);
    if (return_code != 0) perror("ioctl");

    memcpy(packet->source_mac, interface_request.ifr_hwaddr.sa_data, sizeof(packet->source_mac));
    memcpy(packet->ethertype, ARP_ETHERTYPE, sizeof(ARP_ETHERTYPE));
    // ========================================================================

    // Fill ARP Packet ========================================================
    memcpy(packet->htype, ETHERNET_HTYPE, sizeof(ETHERNET_HTYPE));
    memcpy(packet->ptype, IPV4_PTYPE, sizeof(IPV4_PTYPE));
    packet->hlen = ETHERNET_HLEN;
    packet->plen = IPV4_PLEN;
    memcpy(packet->oper, OP_REQUEST, sizeof(OP_REQUEST));
    memcpy(packet->sha, interface_request.ifr_hwaddr.sa_data, sizeof(packet->sha));

    // Get address of the device
    return_code = ioctl(socket_fd, SIOCGIFADDR, &interface_request);
    if (return_code != 0) perror("ioctl");

    struct sockaddr_in* socket_internet_address = (struct sockaddr_in*)&interface_request.ifr_addr;
    memcpy(packet->spa, &(socket_internet_address->sin_addr.s_addr), sizeof(unsigned long));

    // packet->tha is ignored in request
    // packet->tpa will be filled below
    // ========================================================================

    char ip[MAX_IP_SIZE];
    while (scanf("%s", ip) != EOF) {
        // Get IP and put it as target's address
        struct sockaddr_in target_internet_address;
        inet_aton(ip, &target_internet_address.sin_addr);
        memcpy(packet->tpa, &target_internet_address.sin_addr.s_addr, sizeof(unsigned long));

        // Send ARP request to device with unknown MAC address
        return_code = sendto(socket_fd, packet_buffer, sizeof(packet_buffer), 0, (struct sockaddr*)&socket_address, sizeof(socket_address));
        if (return_code == -1) perror("sendto");

        char reply_packet_buffer[sizeof(struct framed_packet)];

        while (1) {
            // Get response
            return_code = recv(socket_fd, reply_packet_buffer, sizeof(reply_packet_buffer), 0);
            if (return_code == -1) perror("recv");

            struct framed_packet* reply_packet = (void*)reply_packet_buffer;

            // Check if packet is for us
            return_code = ioctl(socket_fd, SIOCGIFHWADDR, &interface_request);
            if (return_code != 0) perror("ioctl");

            if (memcmp(reply_packet->tha, interface_request.ifr_hwaddr.sa_data, sizeof(reply_packet->tha))) {
                continue;
            }

            // Print MAC address of device
            for (size_t i = 0; i < sizeof(reply_packet->sha); ++i) {
                printf("%02x", reply_packet->sha[i]);

                if (i + 1 != sizeof(reply_packet->sha)) {
                    printf(":");
                }
            }
            printf("\n");

            break;
        }
    }

    // Close socket
    return_code = close(socket_fd);
    if (return_code != 0) perror("close");

    return 0;
}