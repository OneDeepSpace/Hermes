#pragma once

#include <boost/asio/ip/udp.hpp>

namespace network::service
{
    class IReceiver
    {
    public:
        virtual void process() = 0;

        // http://www.gotw.ca/publications/mill18.htm
        virtual ~IReceiver() {};

    private:
        virtual inline std::size_t isDataReady(const boost::asio::ip::udp::socket& socket) = 0;
    };
}


