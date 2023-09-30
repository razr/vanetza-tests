#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <net/if.h>
#include <arpa/inet.h>

#ifndef ETH_P_IP
#define ETH_P_IP 0x0800
#endif

/* Function to parse MAC address */
int parseMAC(const char* mac_str, unsigned char* mac) {
    return sscanf(mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                  &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <interface> <source_ip> <dest_ip> <dest_mac>\n", argv[0]);
        return 1;
    }

    /* Create a raw socket using AF_PACKET */
    int raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETHERTYPE_IP));

    if (raw_socket == -1) {
        perror("Failed to create raw socket");
        return 1;
    }

    /* Specify the network interface to use */
    char* interface_name = argv[1];

    /* Prepare the sockaddr_ll structure */
    struct sockaddr_ll socket_address;
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sll_family = AF_PACKET;
    socket_address.sll_protocol = htons(ETHERTYPE_IP);
    socket_address.sll_ifindex = if_nametoindex(interface_name);

    if (socket_address.sll_ifindex == 0) {
        perror("Failed to retrieve interface index");
        close(raw_socket);
        return 1;
    }

    /* Destination MAC address */
    unsigned char dest_mac[6];
    if (parseMAC(argv[4], dest_mac) != 6) {
        fprintf(stderr, "Invalid destination MAC address format\n");
        close(raw_socket);
        return 1;
    }

    memcpy(socket_address.sll_addr, dest_mac, 6);
    socket_address.sll_halen = 6;

    /* Prepare the IP packet */
    struct ip ip_header;
    memset(&ip_header, 0, sizeof(ip_header));
    ip_header.ip_hl = 5; /* Header length in 32-bit words */
    ip_header.ip_v = 4;  /* IPv4 */
    ip_header.ip_tos = 0; /* Type of Service (0 for default) */
    ip_header.ip_len = sizeof(ip_header); /* Total length of IP packet */
    ip_header.ip_id = 0; /* Identification (0 for now) */
    ip_header.ip_off = 0; /* Fragment offset (0 for now) */
    ip_header.ip_ttl = 64; /* Time to Live */
    ip_header.ip_p = IPPROTO_RAW; /* Protocol (RAW) */
    ip_header.ip_sum = 0; /* Checksum (0 for now) */
    if (inet_pton(AF_INET, argv[2], &(ip_header.ip_src)) != 1 ||
        inet_pton(AF_INET, argv[3], &(ip_header.ip_dst)) != 1) {
        perror("Failed to parse source or destination IP address");
        close(raw_socket);
        return 1;
    }

    /* Message to send */
    const char* message = "Hello, World!";

    /* Calculate IP checksum */
    ip_header.ip_sum = 0; /* Reset checksum field to 0 */
    uint32_t sum = 0;
    uint16_t *ip_header_short = (uint16_t *)&ip_header;
    for (size_t i = 0; i < sizeof(ip_header) / 2; ++i) {
        sum += ip_header_short[i];
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    ip_header.ip_sum = ~sum;

    /* Prepare the complete packet */
    char packet[sizeof(ip_header) + strlen(message)];
    memcpy(packet, &ip_header, sizeof(ip_header));
    memcpy(packet + sizeof(ip_header), message, strlen(message));

    /* Send the packet */
    if (sendto(raw_socket, packet, sizeof(packet), 0, (struct sockaddr*)&socket_address, sizeof(socket_address)) == -1) {
        perror("Failed to send message");
        close(raw_socket);
        return 1;
    }

    printf("Message sent successfully!\n");

    /* Close the socket */
    close(raw_socket);

    return 0;
}

