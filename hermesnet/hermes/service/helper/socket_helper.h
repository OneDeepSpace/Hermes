
#pragma once

#include <hermes/common/types.h>

using namespace network::types;

namespace network::service::helper
{
    // Создать и подготовить сокет на указанном порту
    std::optional<net::ip::udp::socket> prepareSocket(net::io_service& ios, boost::system::error_code& ec, std::uint16_t port)
    {
        const std::string& address = "127.0.0.1";   // todo: get local ip address
        net::ip::udp::endpoint endpoint(net::ip::address::from_string(address), port);
        net::ip::udp::socket sock(ios);

        sock.open(endpoint.protocol(), ec);
        if (ec.failed() or !sock.is_open()) {
            return std::nullopt;
        }

        sock.bind(endpoint, ec);
        if (ec.failed()) {
            return std::nullopt;
        }

        sock.non_blocking(true);

        sock.set_option(net::ip::udp::socket::send_buffer_size(SOCK_BUF_SIZE));
        sock.set_option(net::ip::udp::socket::receive_buffer_size(SOCK_BUF_SIZE));
        sock.set_option(net::ip::udp::socket::reuse_address(true));
        const std::uint32_t dummyBoostTemplateParam {0};
        sock.set_option(net::ip::udp::socket::linger(false, dummyBoostTemplateParam));

        return sock;
    }

// Закрыть указанный сокет
    static void closeSocket(net::ip::udp::socket& sock)
    {
        if (sock.is_open())
        {
            boost::system::error_code ec;
            auto ep = sock.local_endpoint(ec);

            // close gracefully
            // < looks like not for the UDP overall ¯\_(ツ)_/¯ >
            sock.shutdown(net::ip::udp::socket::shutdown_both, ec);
            sock.close();
        }
    }
}
