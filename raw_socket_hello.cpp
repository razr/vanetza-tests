#include <iostream>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
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
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <interface_name> <source_mac> <dest_mac>" << std::endl;
        return 1;
    }

    const char* interfaceName = argv[1];
    const char* sourceMacStr = argv[2];
    const char* destMacStr = argv[3];

    int rawSocket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (rawSocket == -1) {
        if (errno == EAFNOSUPPORT) {
            std::cerr << "Raw sockets with AF_PACKET are not supported." << std::endl;
        } else {
            std::cerr << "Socket creation error: " << strerror(errno) << std::endl;
        }
        return 1;
    }

    // Construct the Ethernet frame
    struct ethhdr etherHeader;
    struct sockaddr_ll sa;

    memset(&etherHeader, 0, sizeof(etherHeader));

    // Parse source and destination MAC addresses using sscanf
    if (sscanf(sourceMacStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &etherHeader.h_source[0], &etherHeader.h_source[1], &etherHeader.h_source[2],
               &etherHeader.h_source[3], &etherHeader.h_source[4], &etherHeader.h_source[5]) != 6 ||
        sscanf(destMacStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &etherHeader.h_dest[0], &etherHeader.h_dest[1], &etherHeader.h_dest[2],
               &etherHeader.h_dest[3], &etherHeader.h_dest[4], &etherHeader.h_dest[5]) != 6) {
        std::cerr << "Invalid MAC address format." << std::endl;
        close(rawSocket);
        return 1;
    }

    etherHeader.h_proto = htons(ETH_P_IP);

    // Set the destination interface
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

    // Send the packet
    ssize_t bytesSent = sendto(rawSocket, &etherHeader, sizeof(etherHeader), 0, (struct sockaddr*)&sa, sizeof(sa));

    if (bytesSent == -1) {
        std::cerr << "Failed to send message: " << strerror(errno) << std::endl;
    } else {
        std::cout << "Message sent successfully." << std::endl;
    }

    close(rawSocket);
    return 0;
}

