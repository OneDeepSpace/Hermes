
#include <iostream>

#include <boost/asio.hpp>
#include <hermes/common/common.h>
#include <hermes/message/message.hpp>
#include <hermes/common/structures.h>

namespace
{
#define LOG(text) \
    utility::logger::Logger::getInstance().log(EModule::CLIENT, (text));
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

    namespace net = boost::asio;
    using namespace network;

    net::io_service ios_;
    const std::uint16_t port { 10'000 };
    const std::string& address = "127.0.0.1";   // todo: get local ip address (such as ifconfig->inet)
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

        socket.set_option(net::ip::udp::socket::send_buffer_size(SOCK_BUF_SIZE));
        socket.set_option(net::ip::udp::socket::receive_buffer_size(SOCK_BUF_SIZE));
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

        // with struct point_t
        using namespace message;
        message_block_t<message_id> msg_point;
        msg_point.header.type.action = message_id::action_t::PING;
        msg_point.header.access_code = 0xEA;
        msg_point.header.uuid = 42;
        //msg.insert(pStr, payload.size()); // todo: make helper function for string
        test::point_t point;
        point.x = 111;
        point.y = 333;
        msg_point.insert(point);

//        // with text
//        message_block_t<message_id> msg_text;
//        msg_text.header.type.action = message_id::action_t::CHAT_MESSAGE;
//        msg_text.header.access_code = 0xEA;
//        msg_text.header.uuid = 121;
//
//        const char* s {"[test udp message]"};
//        msg_text.insert(s, strlen(s));

        auto server_endpoint { net::ip::udp::endpoint(net::ip::address::from_string("127.0.0.1"), SERVER_IN_PORT)};
        {
            std::stringstream ss;
            ss << "try to send messages to " << server_endpoint;
            LOG(ss.str().c_str())
        }

        // send point
        auto wrapper_p { boost::asio::buffer(&msg_point, sizeof(msg_point)) };
        auto sent = socket.send_to(wrapper_p, server_endpoint);

        if (sent > 0)
        {
            std::stringstream ss;
            ss << "successful sent [" << sent << "] bytes";
            LOG(ss.str().c_str())
        }

//        // send text
//        auto wrapper_t { boost::asio::buffer(&msg_text, sizeof(msg_text)) };
//        sent = socket.send_to(wrapper_t, server_endpoint);
//
//        if (sent > 0)
//        {
//            std::stringstream ss;
//            ss << "successful sent [" << sent << "] bytes";
//            LOG(ss.str().c_str())
//        }
    }

    {
        std::stringstream ss;
        ss << "done, shutdown";
        LOG(ss.str().c_str())
    }

    socket.close();

    return 0;
}
