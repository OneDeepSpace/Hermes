
#include "receiver.h"

#include <sstream>
#include <iomanip>
#include <hermes/log/log.h>
#include <hermes/common/structures.h>
#include <hermes/message/message.hpp>

using namespace network;
using namespace network::message;
using namespace utility::logger;

namespace
{   // right local LOG macro for this entire module
    #define LOG(text) \
            Logger::getInstance().log(EModule::RECEIVER, (text));
}

DataReceiver::DataReceiver()
{
    LOG_REGISTER_MODULE(EModule::RECEIVER)

    std::stringstream ss;
    ss << "create DataReceiver with access code: " << std::to_string(SERVER_ACCESS_CODE);
    LOG(ss.str().c_str())
}

// ...todo: перенести метод в другой модуль
inline bool DataReceiver::validateData(std::vector<std::uint8_t>& data, std::uint8_t code) {
    return (code == data[ACCESS_BYTE_POS]) and (std::uint8_t(END_MESSAGE_BYTE) == data[END_MESSAGE_BYTE_POS]);
}

inline std::size_t DataReceiver::checkAvailableData(const Socket& sock)
{
    boost::system::error_code ec;
    std::size_t bytes = sock.available(ec);
    if (!static_cast<bool>(bytes) or ec.failed()) {
        return 0;
    }
    return bytes;
}

std::vector<DataReceiver::AssociatedChunk>
DataReceiver::readFromEntrySocket(Socket& socket, std::uint8_t code)
{
    auto bytes = checkAvailableData(socket);
    if (0 == bytes) return {};

    std::vector<AssociatedChunk> result;
    result.reserve(32);

    const auto flags {0};
    boost::system::error_code ec;
    net::ip::udp::endpoint remote_endpoint;

    message::message_block_t<message_id> msg {};
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

        if (msg.header.type.action == message::message_id::action_t::PING) {
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
DataReceiver::readFromClientSockets(std::vector<Socket>& sockets, std::vector<std::size_t>& sizes, std::vector<std::uint8_t>& codes)
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

        if (MESSAGE_SIZE > received) {
            reset(chunk, received);
            continue;
        }

        if (!validateData(chunk, codes[i])) {
            continue;
        }

        extract(chunk);
    } // loop

    return result;
}

void DataReceiver::processIncomingMessages(Entry& entry, Clients& clients, std::vector<std::uint8_t>& incMsgBuffer)
{
    // process messages for server (new clients, management services and etc)
    auto associatedChunks = readFromEntrySocket(entry.in, entry.accessCode);
    // ...logic

    // process message from clients (validated and connected clients)
    auto& ref = clients.vIn;
    std::vector<std::size_t> sizes;
    sizes.reserve(ref.size());

    for (auto& s : ref)
        sizes.push_back(checkAvailableData(s));

    auto chunks = readFromClientSockets(ref, sizes, clients.vAccessCodes);
    // ...logic
}
