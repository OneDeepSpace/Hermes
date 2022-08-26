
#pragma once

#include <boost/noncopyable.hpp>

#include <hermes/netloop/netloop.h>
#include <hermes/common/structures.h>

namespace network::service
{
#ifndef ASIO_TYPEDEF
    #define ASIO_TYPEDEF
    namespace net = boost::asio;
#endif

    struct Context
    {
        std::uint16_t outPort { 7000 };
        std::uint16_t inPort  { 7001 };
    };

    class Server : public boost::noncopyable
    {
    private:
        net::io_service ios_;
        class Context   context_ {};
        // Event event_;
        class Entry     entry_;
        class Clients   clients_;
        class NetLoop   netloop_;

        // todo: make special buffer class for message exchange logic (read/write flags) with event class (todo too)
        std::vector<std::uint8_t> incomingBuffer;
        std::vector<std::uint8_t> outcomingBuffer;

    private:
        bool init(std::pair<std::uint16_t, std::uint16_t> ports);

    public:
        Server() noexcept;
        virtual ~Server();

        bool start(std::pair<std::uint16_t, std::uint16_t> ports);
        bool stop();

    };  // server
}   // network
