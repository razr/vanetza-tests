#include <iostream>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <netinet/if_ether.h> // Include this header for ETH_P_ALL
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <unistd.h>

#ifndef ETH_P_ALL
#define ETH_P_ALL NET_ETH_P_ALL
#endif

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <interface_name>" << std::endl;
        return 1;
    }

    const char* interfaceName = argv[1];

    int rawSocket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (rawSocket == -1) {
        if (errno == EAFNOSUPPORT) {
            std::cerr << "Raw sockets with AF_PACKET are not supported." << std::endl;
        } else {
            std::cerr << "Socket creation error: " << strerror(errno) << std::endl;
        }
    } else {
        // Bind the socket to the specified network interface
        struct sockaddr_ll sa;
        struct ifreq ifr;

        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, interfaceName, IFNAMSIZ - 1);
        if (ioctl(rawSocket, SIOCGIFINDEX, &ifr) == -1) {
            std::cerr << "Failed to get interface index: " << strerror(errno) << std::endl;
            close(rawSocket);
            return 1;
        }

        sa.sll_family = AF_PACKET;
        sa.sll_protocol = htons(ETH_P_ALL);
        sa.sll_ifindex = ifr.ifr_ifindex;

        if (bind(rawSocket, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
            std::cerr << "Failed to bind socket to interface: " << strerror(errno) << std::endl;
            close(rawSocket);
            return 1;
        }

        std::cout << "Raw socket created and bound successfully with AF_PACKET." << std::endl;
        // You can use the rawSocket for packet capturing here.
        // Don't forget to close the socket when you're done.
        close(rawSocket);
    }

    return 0;
}

