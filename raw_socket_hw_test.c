#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef ETH_P_ALL
#define ETH_P_ALL 0x03
#endif

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <interface_name>\n", argv[0]);
        return 1;
    }

    const char* interfaceName = argv[1];

    int raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (raw_socket == -1) {
        perror("Failed to create raw socket");
        return 1;
    }

    struct ifreq request_in;
    memset(&request_in, 0, sizeof(struct ifreq));
    strncpy(request_in.ifr_name, interfaceName, IFNAMSIZ - 1);

    if (ioctl(raw_socket, SIOCGIFINDEX, &request_in) == -1) {
        perror("Failed to get interface index");
        close(raw_socket);
        return 1;
    }

    struct sockaddr_ll socket_address;
    memset(&socket_address, 0, sizeof(struct sockaddr_ll));
    socket_address.sll_family = AF_PACKET;
    socket_address.sll_protocol = htons(ETH_P_ALL);
    socket_address.sll_ifindex = request_in.ifr_ifindex;

    if (bind(raw_socket, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) == -1) {
        perror("Failed to bind raw socket");
        close(raw_socket);
        return 1;
    }

    struct ifreq request;
    memset(&request, 0, sizeof(struct ifreq));

    // Use ioctl to retrieve the interface index
    if (ioctl(raw_socket, SIOCGIFINDEX, &request) == -1) {
        perror("Failed to get interface index");
        close(raw_socket);
        return 1;
    }

    // Use ioctl to retrieve the hardware address (MAC address) based on the interface index
    struct ifreq mac_request;
    memset(&mac_request, 0, sizeof(struct ifreq));
    mac_request.ifr_ifindex = request.ifr_ifindex;

    if (ioctl(raw_socket, SIOCGIFHWADDR, &mac_request) == -1) {
        perror("Failed to get MAC address");
        close(raw_socket);
        return 1;
    }

    // Retrieve and print the MAC address
    printf("MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n",
           (unsigned char)mac_request.ifr_hwaddr.sa_data[0],
           (unsigned char)mac_request.ifr_hwaddr.sa_data[1],
           (unsigned char)mac_request.ifr_hwaddr.sa_data[2],
           (unsigned char)mac_request.ifr_hwaddr.sa_data[3],
           (unsigned char)mac_request.ifr_hwaddr.sa_data[4],
           (unsigned char)mac_request.ifr_hwaddr.sa_data[5]);

    close(raw_socket);

    return 0;
}

