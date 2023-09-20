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

    struct ifreq request;
    memset(&request, 0, sizeof(struct ifreq));
    strncpy(request.ifr_name, interfaceName, IFNAMSIZ - 1);

    if (ioctl(raw_socket, SIOCGIFINDEX, &request) == -1) {
        perror("Failed to get interface index");
        close(raw_socket);
        return 1;
    }

    struct sockaddr_ll socket_address;
    memset(&socket_address, 0, sizeof(struct sockaddr_ll));
    socket_address.sll_family = AF_PACKET;
    socket_address.sll_protocol = htons(ETH_P_ALL);
    socket_address.sll_ifindex = request.ifr_ifindex;

    if (bind(raw_socket, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) == -1) {
        perror("Failed to bind raw socket");
        close(raw_socket);
        return 1;
    }

    socklen_t addr_len = sizeof(struct sockaddr_ll);
    if (getsockname(raw_socket, (struct sockaddr*)&socket_address, &addr_len) == -1) {
        perror("Failed to get socket name");
        close(raw_socket);
        return 1;
    }

    printf("sll_family: %d\n", socket_address.sll_family);
    printf("sll_protocol: %d\n", ntohs(socket_address.sll_protocol));
    printf("sll_ifindex: %d\n", socket_address.sll_ifindex);
    printf("sll_hatype: %d\n", socket_address.sll_hatype);
    printf("sll_pkttype: %d\n", socket_address.sll_pkttype);
    printf("sll_halen: %d\n", socket_address.sll_halen);
    printf("sll_addr: ");
    for (int i = 0; i < socket_address.sll_halen; ++i) {
        printf("%02x", socket_address.sll_addr[i]);
        if (i < socket_address.sll_halen - 1) {
            printf(":");
        }
    }
    printf("\n");

    close(raw_socket);

    return 0;
}

