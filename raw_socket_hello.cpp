#include <iostream>
#include <cstring>
#include <cstdlib>  // for the exit() function
#include <unistd.h> // for close() function
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <net/if.h> // for if_nametoindex
#include <arpa/inet.h> // for inet_pton

// Function to parse MAC address
bool parseMAC(const char* mac_str, unsigned char* mac) {
    return sscanf(mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                  &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == 6;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <interface> <source_ip> <dest_ip> <dest_mac>" << std::endl;
        return 1;
    }

    // Create a raw socket using AF_PACKET
    int raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP)); // Use ETH_P_IP for IPv4

    if (raw_socket == -1) {
        perror("Failed to create raw socket");
        return 1;
    }

    // Specify the network interface to use
    std::string interface_name = argv[1];

    // Prepare the sockaddr_ll structure
    struct sockaddr_ll socket_address;
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sll_family = AF_PACKET;
    socket_address.sll_protocol = htons(ETH_P_IP); // Use ETH_P_IP for IPv4
    socket_address.sll_ifindex = if_nametoindex(interface_name.c_str());

    if (socket_address.sll_ifindex == 0) {
        perror("Failed to retrieve interface index");
        close(raw_socket);
        return 1;
    }

    // Destination MAC address
    unsigned char dest_mac[6];
    if (!parseMAC(argv[4], dest_mac)) {
        std::cerr << "Invalid destination MAC address format" << std::endl;
        close(raw_socket);
        return 1;
    }

    memcpy(socket_address.sll_addr, dest_mac, 6);
    socket_address.sll_halen = 6;

    // Prepare the IP packet
    struct ip ip_header;
    memset(&ip_header, 0, sizeof(ip_header));
    ip_header.ip_hl = 5; // Header length in 32-bit words
    ip_header.ip_v = 4;  // IPv4
    ip_header.ip_tos = 0; // Type of Service (0 for default)
    ip_header.ip_len = sizeof(ip_header); // Total length of IP packet
    ip_header.ip_id = 0; // Identification (0 for now)
    ip_header.ip_off = 0; // Fragment offset (0 for now)
    ip_header.ip_ttl = 64; // Time to Live
    ip_header.ip_p = IPPROTO_RAW; // Protocol (RAW)
    ip_header.ip_sum = 0; // Checksum (0 for now)
    if (inet_pton(AF_INET, argv[2], &(ip_header.ip_src)) != 1 ||
        inet_pton(AF_INET, argv[3], &(ip_header.ip_dst)) != 1) {
        perror("Failed to parse source or destination IP address");
        close(raw_socket);
        return 1;
    }

    // Message to send
    std::string message = "Hello, World!";

    // Calculate IP checksum
    ip_header.ip_sum = 0; // Reset checksum field to 0
    uint32_t sum = 0;
    uint16_t *ip_header_short = (uint16_t *)&ip_header;
    for (size_t i = 0; i < sizeof(ip_header) / 2; ++i) {
        sum += ip_header_short[i];
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    ip_header.ip_sum = ~sum;

    // Prepare the complete packet
    std::string packet(sizeof(ip_header) + message.size(), 0);
    memcpy(&packet[0], &ip_header, sizeof(ip_header));
    memcpy(&packet[sizeof(ip_header)], message.c_str(), message.size());

    // Send the packet
    if (sendto(raw_socket, packet.c_str(), packet.size(), 0, (struct sockaddr*)&socket_address, sizeof(socket_address)) == -1) {
        perror("Failed to send message");
        close(raw_socket);
        return 1;
    }

    std::cout << "Message sent successfully!" << std::endl;

    // Close the socket
    close(raw_socket);

    return 0;
}

