#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/generic/raw_protocol.hpp>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netpacket/packet.h>

#ifndef ETH_P_ALL
#define ETH_P_ALL 0
#endif

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <interface_name>" << std::endl;
        return 1;
    }

    const char* interfaceName = argv[1]; // Get the interface name from the command line argument

    try {
        boost::asio::io_service io_service;
        boost::asio::generic::raw_protocol raw_protocol(AF_PACKET, SOCK_RAW);
        boost::asio::generic::raw_protocol::socket raw_socket(io_service, raw_protocol);

        int local_socket = ::socket(AF_LOCAL, SOCK_DGRAM, 0);

        ifreq request;

        std::memset(&request, 0, sizeof(ifreq));
        std::strncpy(request.ifr_name, interfaceName, IF_NAMESIZE);
        request.ifr_name[IF_NAMESIZE - 1] = '\0';
        ::ioctl(local_socket, SIOCGIFINDEX, &request);

        sockaddr_ll socket_address = {0};
        socket_address.sll_family = AF_PACKET;
        socket_address.sll_protocol = htons(ETH_P_ALL);
        socket_address.sll_ifindex = request.ifr_ifindex;

        boost::asio::generic::raw_protocol::endpoint endpoint(&socket_address, sizeof(sockaddr_ll));
        raw_socket.bind(endpoint);

        boost::asio::generic::raw_protocol::endpoint local_endpoint = raw_socket.local_endpoint();

    } catch (const boost::system::system_error& e) {
        std::cerr << "Boost System Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

