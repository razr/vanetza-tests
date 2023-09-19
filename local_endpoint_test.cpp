#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/generic/raw_protocol.hpp>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <interface_name>" << std::endl;
        return 1;
    }

    const char* interfaceName = argv[1]; // Get the interface name from the command line argument

    boost::asio::io_service io_service;
    boost::asio::generic::raw_protocol raw_protocol(AF_PACKET, SOCK_RAW);
    boost::asio::generic::raw_protocol::socket raw_socket(io_service, raw_protocol);
    boost::asio::generic::raw_protocol::endpoint endpoint = raw_socket.local_endpoint();

    int local_socket = ::socket(AF_LOCAL, SOCK_DGRAM, 0);

    ifreq request;

    std::memset(&request, 0, sizeof(ifreq));
    std::strncpy(request.ifr_name, interfaceName, IF_NAMESIZE);
    request.ifr_name[IF_NAMESIZE - 1] = '\0';
    ::ioctl(local_socket, SIOCGIFINDEX, &request);
    std::cout << "Interface index for " << interfaceName << ": " << request.ifr_ifindex << std::endl;

    return 0;
}

