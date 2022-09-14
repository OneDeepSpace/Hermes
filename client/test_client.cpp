
#include <iostream>

#include <boost/asio.hpp>

#include <hermes/log/log.h>
#include <hermes/common/types.h>
#include <hermes/common/common.h>
#include <hermes/message/service_type_id.h>
#include <hermes/message/message_generator.h>
#include <hermes/common/structures.h>

#include <hermes/message/objects/ping.h>

#include "../server/chat_type_id.h"

using namespace network;
using namespace network::message;
using namespace network::message::id;
using namespace network::message::object;
using namespace utility::logger;

namespace
{
#undef LOG
#define LOG(text) utility::logger::Logger::getInstance().log(EModule::CLIENT, (text));
}

int main()
{
    // config --- todo: add boost::program_options + {unit}.ini
    {
        const char* appName  { "client" };
        const char* p7Config { "/P7.Sink=Console /P7.On=1 /P7.Pool=1024 /P7.Format=\"{%cn}{%mn}[%tm][%lv] %ms\"" };

        Logger::init(p7Config, appName);
        LOG_REGISTER_MODULE(EModule::MAIN)
        LOG_REGISTER_MODULE(EModule::MESSAGE)
    }

    using namespace network;

    net::io_service ios_;
    const std::uint16_t port { 16'000 };
    const std::string& address = "127.0.0.1";   // todo: get local ip address (such as ifconfig->inet)
    net::ip::udp::resolver resolver(ios_);

    net::ip::udp::endpoint local_endpoint(net::ip::address::from_string(address), port);

    boost::system::error_code ec;
    net::ip::udp::socket socket(ios_);

    {
        {
            std::stringstream ss;
            ss << "prepare socket on " << local_endpoint;
            LOG(ss.str().c_str())
        }

        socket.open(net::ip::udp::v4(), ec);
        if (ec.failed() or !socket.is_open())
        {
            std::stringstream ss;
            ss << "error while opening socket: " << local_endpoint << " " << ec.message();
            LOG(ss.str().c_str())
        }

        socket.bind(local_endpoint, ec);
        if (ec.failed() or !socket.is_open())
        {
            std::stringstream ss;
            ss << "error while binding socket: " << local_endpoint << " " << ec.message();
            LOG(ss.str().c_str())
        }

        socket.non_blocking(true);

        socket.set_option(net::ip::udp::socket::send_buffer_size(8192));
        socket.set_option(net::ip::udp::socket::receive_buffer_size(8192));
        socket.set_option(net::ip::udp::socket::reuse_address(true));
        const std::uint32_t dummyBoostTemplateParam {0};
        socket.set_option(net::ip::udp::socket::linger(false, dummyBoostTemplateParam));
    }

    {   // generate messages
        {
            std::stringstream ss;
            ss << "generate messages";
            LOG(ss.str().c_str())
        }

        const std::string payload {"[test udp message]"};
        const char* pStr = payload.data();

        // ------------- with struct point_t -------------
        Header<ServiceType> header;
        header.type.action = ServiceType::EServiceAction::SERVICE_ACT_PING;
        header.uuid = 0x42;
        header.block_num = 7;
        header.block_count = 10;
        Body body;

        Datagram<ServiceType> datagram(std::move(header), std::move(body));

        MPing ping;
        ping.setStart();
        std::this_thread::sleep_for(std::chrono::milliseconds(144));
        ping.setEnd();

        auto[ok, wrote, free] = datagram.BodyRef().write(ping, sizeof(ping));

        helper::prepareDatagram(datagram, 0xEA, 0xFF);
        // --------------------------------- -------------

        auto server_endpoint { net::ip::udp::endpoint(net::ip::address::from_string("127.0.0.1"), 7000)};
        auto resolved_endpoint { *resolver.resolve(server_endpoint, ec).begin() };
        {
            std::stringstream ss;
            ss << "try to send messages to " << server_endpoint;
            LOG(ss.str().c_str())
        }

        {
            std::stringstream ss;
            ss  << ping
                << "sizeof - " << sizeof(ping) << "\n"
                << datagram
                << "sizeof - " << sizeof(datagram) << "\n";
            LOG(ss.str().c_str())
        }


        auto wrap { boost::asio::buffer(&datagram, sizeof(datagram)) };
        auto bytes = socket.send_to(wrap, server_endpoint, 0, ec);

        if (ec.failed())
        {
            LOG(ec.message().c_str())
        }

        if (bytes > 0)
        {
            std::stringstream ss;
            ss << "successful sent [" << bytes << "] bytes";
            LOG(ss.str().c_str())
        }
    }

    {
        std::stringstream ss;
        ss << "done, shutdown";
        LOG(ss.str().c_str())
    }

    if (socket.is_open())
    {
        socket.close();
    }

    return 0;
}
