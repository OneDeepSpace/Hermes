#pragma once

#include "interface/isender.h"
#include <boost/noncopyable.hpp>

namespace network::service
{
    class ServerDataSender final : public ISender, public boost::noncopyable
    {
    public:
        virtual ~ServerDataSender() = default;

        void process() final { };
    };
}
