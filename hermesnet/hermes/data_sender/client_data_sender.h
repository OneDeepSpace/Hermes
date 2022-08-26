#pragma once

#include <cstdint>

#include "interface/isender.h"
#include <hermes/common/types.h>
#include <hermes/common/structures.h>

#include <boost/noncopyable.hpp>
#include <boost/asio/ip/udp.hpp>

namespace network::service
{
    class ClientDataSender final : public ISender, public boost::noncopyable
    {
    public:
        virtual ~ClientDataSender() = default;

        void process() final { };

    };
}


