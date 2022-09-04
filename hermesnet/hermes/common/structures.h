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

        net::ip::udp::socket    in;         // in service udp port
        net::ip::udp::socket    out;        // out service udp port
        std::uint8_t            accessCode; // access byte for each service datagram
    };

    typedef Entry Client;

    /*
     * Structure of Arrays
     */
    struct Clients
    {
        std::vector<net::ip::udp::socket>   vIn;            // client's in udp port
        std::vector<net::ip::udp::socket>   vOut;           // client's out udp port
        std::vector<net::ip::udp::endpoint> vEndpoints;     // endpoint associated with each client
        std::vector<std::uint32_t>          vUUIDs;         // unique id for each client
        std::vector<std::uint8_t>           vAccessCodes;   // client's unique access byte (each connection)

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

