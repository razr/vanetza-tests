#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <unistd.h>

#ifndef ETH_P_ALL
#define ETH_P_ALL 0x0003
#endif

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <interface_name>\n", argv[0]);
        return 1;
    }

    const char* interfaceName = argv[1];

    int rawSocket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (rawSocket == -1) {
        if (errno == EAFNOSUPPORT) {
            fprintf(stderr, "Raw sockets with AF_PACKET are not supported.\n");
        } else {
            fprintf(stderr, "Socket creation error: %s\n", strerror(errno));
        }
    } else {
        // Bind the socket to the specified network interface
        struct sockaddr_ll sa;
        struct ifreq ifr;

        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, interfaceName, IFNAMSIZ - 1);
        if (ioctl(rawSocket, SIOCGIFINDEX, &ifr) == -1) {
            fprintf(stderr, "Failed to get interface index: %s\n", strerror(errno));
            close(rawSocket);
            return 1;
        }

        sa.sll_family = AF_PACKET;
        sa.sll_protocol = htons(ETH_P_ALL);
        sa.sll_ifindex = ifr.ifr_ifindex;

        if (bind(rawSocket, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
            fprintf(stderr, "Failed to bind socket to interface: %s\n", strerror(errno));
            close(rawSocket);
            return 1;
        }

        printf("Raw socket created and bound successfully with AF_PACKET.\n");
        // You can use the rawSocket for packet capturing here.
        // Don't forget to close the socket when you're done.
        close(rawSocket);
    }

    return 0;
}

