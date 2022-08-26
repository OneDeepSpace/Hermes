
#include "server_data_receiver.h"

#include <iomanip>
#include <hermes/log/log.h>
#include <hermes/message/message_generator.h>

using namespace network;
using namespace network::service;
using namespace network::types;
using namespace utility::logger;
using namespace network::message::v2;

namespace
{
    #define LOG(text) \
            Logger::getInstance().log(EModule::RECEIVER, (text));
}

ServerDataReceiver::ServerDataReceiver(net::io_service &service, Entry& e, Clients& c)
    : refEntry_(e)
    , refClients_(c)
{
    LOG_REGISTER_MODULE(EModule::RECEIVER)
}

void ServerDataReceiver::process()
{
    // process messages for server (new clients, management services and etc)
    auto associatedChunks = readFromEntrySocket(refEntry_.in, refEntry_.accessCode);
    // ...logic

    // process message from clients (validated and connected clients)
    std::vector<std::size_t> sizes;
    sizes.reserve(refClients_.vIn.size());

    for (auto& s : refClients_.vIn)
        sizes.push_back(isDataReady(s));

    auto chunks = readFromClientSockets(refClients_.vIn, sizes, refClients_.vAccessCodes);
    // ...logic
}

std::size_t ServerDataReceiver::isDataReady(const boost::asio::ip::udp::socket& socket)
{
    boost::system::error_code ec;
    std::size_t bytes = socket.available(ec);
    if (!static_cast<bool>(bytes) or ec.failed()) {
        return 0;
    }
    return bytes;
}

std::vector<ServerDataReceiver::AssociatedChunk>
ServerDataReceiver::readFromEntrySocket(net::ip::udp::socket& socket, std::uint8_t code)
{
    auto bytes = isDataReady(socket);
    if (0 == bytes) return {};

    std::vector<AssociatedChunk> result;
    result.reserve(32);

    const auto flags {0};
    boost::system::error_code ec;
    net::ip::udp::endpoint remote_endpoint;

    message::v2::message_block_t<service_type> msg {};
    auto wrapper { boost::asio::buffer(&msg, 64) };

    std::vector<std::uint8_t> chunk (MESSAGE_SIZE, 0x0);
//    auto wrapper { net::buffer(chunk.data(), chunk.size()) };
    auto received = socket.receive_from(wrapper, remote_endpoint, flags, ec);

    if (ec.failed()) {
        std::stringstream ss;
        ss << "error while reading data from entry socket: " << std::quoted(ec.message());
        LOG(ss.str().c_str())
        return {};
    }

    /*
    if (MESSAGE_SIZE != received) return {};

    if (!validateData(chunk, code)) return {};

    {
        std::stringstream ss;
        ss << "received data test [" << chunk.size() << "]: " << chunk.data() << " from " << remote_endpoint;
        LOG_SERVER(ss.str())
    }
    */

    // [testing section]
    if (received > 0)
    {
        {
            std::stringstream ss;
            ss << "recevied message:\n" << msg << "\n";
            LOG(ss.str().c_str())
        }

        if (msg.header.type.action == message::v2::service_type::action_t::PING) {
            test::point_t point_received;
            msg.extract(point_received);

            std::stringstream ss;
            ss << "received point: " << point_received;
            LOG(ss.str().c_str())
        }

        // .....need helper function for string
//        std::string payload {};
//        const char* pStr = payload.data();
//        msg.extract(pStr, msg.get_payload_size());
//        std::string log("payload: " + payload);
//        LOG(log.c_str())
    }

    result.emplace_back( remote_endpoint, chunk );

    return result;
}

std::vector<std::uint8_t>
ServerDataReceiver::readFromClientSockets(std::vector<net::ip::udp::socket> &sockets, std::vector<std::size_t> &sizes, std::vector<std::uint8_t> &codes)
{
    std::vector<std::uint8_t> result;
    result.reserve(8192);   // todo: MAGIC WORD

    auto reset = [](std::vector<std::uint8_t>& v, std::size_t len) -> void {
        std::memset(static_cast<std::uint8_t*>(v.data()), 0x0, len * sizeof(std::uint8_t));
    };

    auto extract = [&reset, &result](std::vector<std::uint8_t>& chunk) -> void {
        result.insert(std::end(result), std::begin(chunk), std::end(chunk));
        reset(chunk, chunk.size());
    };

    const auto flags {0};
    std::size_t received {0};
    boost::system::error_code ec;
    std::vector<std::uint8_t> chunk (MESSAGE_SIZE, 0x0); // type -> message_block_t<message_id_t>

    const auto count = sockets.size();
    for (std::size_t i = 0; i < count; ++i)
    {
        // little improvement by caching size
        received = sizes[i];
        if (0 == received) continue;

        auto wrapper { boost::asio::buffer(chunk.data(), received) };
        received = sockets[i].receive(wrapper, flags, ec);

        if (ec.failed()) {
            std::stringstream ss;
            ss << "error while reading data from socket [" << sockets[i].local_endpoint().port() << "]: " << std::quoted(ec.message());
            LOG(ss.str().c_str())
            reset(chunk, received);
            continue;
        }

        /*
        if (MESSAGE_SIZE > received) {
            reset(chunk, received);
            continue;
        }

        if (!validateData(chunk, codes[i])) {
            continue;
        }
         */

        extract(chunk);
    } // loop

    return result;
}



//bool ServerDataReceiver::validateData(std::vector<std::uint8_t> &data, std::uint8_t code) {
//    return false;
//}
