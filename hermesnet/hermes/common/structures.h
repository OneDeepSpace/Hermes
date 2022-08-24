#pragma once

#include <boost/asio.hpp>

namespace network
{
#ifndef ASIO_TYPEDEF
#define ASIO_TYPEDEF
    namespace net = boost::asio;
#endif

    /*
     * wrapper for server's in/out udp sockets for interface compatibility
     */
    struct Entry
    {
        explicit Entry(net::io_service& ios)
            : in(ios)
            , out(ios)
        {}

        net::ip::udp::socket in;
        net::ip::udp::socket out;
        std::uint8_t accessCode;
    };

    /*
     * Structure of Arrays
     */
    struct Clients
    {
        std::vector<net::ip::udp::socket> vIn;
        std::vector<net::ip::udp::socket> vOut;
        std::vector<net::ip::udp::endpoint> vEndpoints;
        std::vector<std::uint32_t> vUUIDs;
        std::vector<std::uint8_t> vAccessCodes;

        void reserve(std::uint32_t count) {
            vIn.reserve(count);
            vOut.reserve(count);
            vEndpoints.reserve(count);
            vUUIDs.reserve(count);
            vAccessCodes.reserve(count);
        }
    };

}   // network

namespace test
{
    // test struct
    struct point_t {
        std::uint32_t x { 0 };
        std::uint32_t y { 0 };

        friend std::ostream& operator<< (std::ostream& os, const point_t& p) {
            os << "{" << p.x << ", " << p.y << "}";
            return os;
        }
    };
}

