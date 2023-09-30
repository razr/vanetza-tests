#include <iostream>
#include <iomanip>
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
        // Access the raw data
        const struct sockaddr_ll* ll_addr = reinterpret_cast<const struct sockaddr_ll*>(local_endpoint.data());

        // Print the fields of struct sockaddr_ll
        std::cout << "sll_family: " << ll_addr->sll_family << std::endl;
        std::cout << "sll_protocol: " << ll_addr->sll_protocol << std::endl;
        std::cout << "sll_ifindex: " << ll_addr->sll_ifindex << std::endl;
        std::cout << "sll_hatype: " << ll_addr->sll_hatype << std::endl;
        std::cout << "sll_pkttype: " << static_cast<int>(ll_addr->sll_pkttype) << std::endl;
        std::cout << "sll_halen: " << static_cast<int>(ll_addr->sll_halen) << std::endl;
    
        // Print sll_addr as a series of hexadecimal values
        std::cout << "sll_addr: ";
        for (int i = 0; i < ll_addr->sll_halen; ++i) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ll_addr->sll_addr[i]);
            if (i < ll_addr->sll_halen - 1) std::cout << ":";
        }
        std::cout << std::dec << std::endl;

    } catch (const boost::system::system_error& e) {
        std::cerr << "Boost System Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

