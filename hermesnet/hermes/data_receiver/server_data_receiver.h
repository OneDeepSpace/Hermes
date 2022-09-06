#pragma once

#include <cstdint>

#include "interface/ireceiver.h"
#include <hermes/common/types.h>
#include <hermes/common/structures.h>
#include <hermes/buffers/ring_buffer.h>
#include <hermes/message/message_block.h>

#include <boost/noncopyable.hpp>
#include <boost/asio/ip/udp.hpp>

using namespace network::buffer;
using namespace network::message::v2;

namespace network::service
{
#ifndef ASIO_TYPEDEF
#define ASIO_TYPEDEF
    namespace net = boost::asio;
#endif

    template <typename MessageType>
    class ServerDataReceiver final : public IReceiver, boost::noncopyable
    {
    private:
        // typedef
        using ServiceMessageType = TimedMessage<message_block_t<service_type>>;
        using ConcreteMessageType = TimedMessage<message_block_t<MessageType>>;

    private:
        class Entry&    refEntry_;
        class Clients&  refClients_;

        // кольцевые буферы для входящих сообщений c пометкой времени прибытия
        class MessageBuffer<ServiceMessageType>   serviceInBuf_;
        class MessageBuffer<ConcreteMessageType>  messageInBuf_;

    public:
        explicit ServerDataReceiver(net::io_service& service, Entry& e, Clients& c);
        virtual ~ServerDataReceiver() = default;

        // Обработать входящие сообщения
        void process() final;

    private:
        // Получить количество доступных байт для чтения без блокировки
        inline std::size_t isDataReady(const boost::asio::ip::udp::socket& socket) final;

        // Проверить доступ сообщения
        //inline bool validateData(std::vector<std::uint8_t>& data, std::uint8_t code);

        // Прочитать ассоциированные с удаленной точкой данные с входного сокета сервера
        void readFromEntry(net::ip::udp::socket& socket, std::uint8_t code);
        // Прочитать данные от всех клиентов
        void readFromClients(std::vector<net::ip::udp::socket>& sockets, std::vector<std::size_t>& sizes, std::vector<std::uint8_t>& codes);

    };  // ServerDataReceiver

}   // network::service

// ██████  ███████ ███████ ██ ███    ██ ██ ████████ ██  ██████  ███    ██
// ██   ██ ██      ██      ██ ████   ██ ██    ██    ██ ██    ██ ████   ██
// ██   ██ █████   █████   ██ ██ ██  ██ ██    ██    ██ ██    ██ ██ ██  ██
// ██   ██ ██      ██      ██ ██  ██ ██ ██    ██    ██ ██    ██ ██  ██ ██
// ██████  ███████ ██      ██ ██   ████ ██    ██    ██  ██████  ██   ████

#include <iomanip>
#include <hermes/log/log.h>
#include <hermes/message/message_generator.h>

#include <hermes/common/duration_bench.h>

using namespace network;
using namespace network::service;
using namespace network::types;
using namespace utility::logger;
using namespace network::buffer;
using namespace network::message::v2;

using namespace utility::bench;

namespace
{
#undef  LOG
#define LOG(text) Logger::getInstance().log(EModule::RECEIVER, (text));
}

template<typename MessageType>
ServerDataReceiver<MessageType>::ServerDataReceiver(boost::asio::io_service &service, Entry &e, Clients &c)
        : refEntry_(e)
        , refClients_(c)
        , serviceInBuf_(1024)
        , messageInBuf_(1024)
{
    LOG_REGISTER_MODULE(EModule::RECEIVER)
}

template<typename MessageType>
void ServerDataReceiver<MessageType>::process()
{
    LOG_DURATION("receiver::process")

    // process messages for server (new clients, management services and etc)
    readFromEntry(refEntry_.in, refEntry_.accessCode);
    // ...logic

    // process message from clients (validated and connected clients)
    std::vector<std::size_t> sizes;
    sizes.reserve(refClients_.vIn.size());

    for (auto& s : refClients_.vIn)
        sizes.push_back(isDataReady(s));

    readFromClients(refClients_.vIn, sizes, refClients_.vAccessCodes);
    // ...logic
}

template<typename MessageType>
inline std::size_t ServerDataReceiver<MessageType>::isDataReady(const boost::asio::ip::udp::socket& socket)
{
    boost::system::error_code ec;
    std::size_t bytes = socket.available(ec);
    if (!static_cast<bool>(bytes) or ec.failed()) {
        return 0;
    }
    return bytes;
}

template<typename MessageType>
void ServerDataReceiver<MessageType>::readFromEntry(net::ip::udp::socket& socket, std::uint8_t code)
{
    if (not isDataReady(socket))
        return;

    const auto flags {0};
    boost::system::error_code ec;
    net::ip::udp::endpoint remote_endpoint;

    ServiceMessageType timedMessage;
    auto wrapper { boost::asio::buffer(&timedMessage.message, 64) };

    auto received = socket.receive_from(wrapper, remote_endpoint, flags, ec);

    if (ec.failed()) {
        std::stringstream ss;
        ss << "error while reading data from entry socket: " << std::quoted(ec.message());
        LOG(ss.str().c_str())
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

    // [testing section - begin]
    if (received > 0)
    {
        {
            std::stringstream ss;
            ss  << "recevied message [" << std::chrono::system_clock::to_time_t(timedMessage.arrivedTime) <<  "]:\n"
                << timedMessage.message << "\n";

            LOG(ss.str().c_str())
        }

        if (timedMessage.message.header.type.action == message::v2::service_type::action_t::PING) {
            test::point_t point_received;
            timedMessage.message.extract(point_received);

            std::stringstream ss;
            ss << "received point: " << point_received;
            LOG(ss.str().c_str())
        }
    }
    // [testing section - end]

    if (not serviceInBuf_.full())
    {
        serviceInBuf_.storeElem(std::forward<decltype(timedMessage)>(timedMessage));
    }
}

template<typename MessageType>
void ServerDataReceiver<MessageType>::readFromClients(std::vector<net::ip::udp::socket> &sockets, std::vector<std::size_t> &sizes, std::vector<std::uint8_t> &codes)
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
}

