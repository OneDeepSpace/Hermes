#pragma once

#include <cstdint>

#include "interface/ireceiver.h"
#include <hermes/common/types.h>
#include <hermes/common/structures.h>

#include <boost/noncopyable.hpp>
#include <boost/asio/ip/udp.hpp>

namespace network::service
{
#ifndef ASIO_TYPEDEF
    #define ASIO_TYPEDEF
    namespace net = boost::asio;
#endif

    class ClientDataReceiver final : public IReceiver, boost::noncopyable
    {
    public:
        ClientDataReceiver() = default;
        virtual ~ClientDataReceiver() = default;

        // Обработать входящие сообщения
        void process() final {};

    private:
        inline std::size_t isDataReady(const boost::asio::ip::udp::socket& socket) final {
            return 0;
        };
    };

}   // network
