
#pragma once

#include <hermes/common/duration_bench.h>

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
        std::uint16_t inPort  { 7000 };
        std::uint16_t outPort { 7001 };
    };

    template <typename MessageType>
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

// ********************************* IMPLEMENTATION **********************************


#include <memory>


#include <hermes/log/log.h>
#include <hermes/common/types.h>
#include <hermes/message/datagram.h>
#include <hermes/service/helper/socket_helper.h>
#include <hermes/data_sender/server_data_sender.h>
#include <hermes/data_receiver/server_data_receiver.h>

using namespace utility::logger;
using namespace network::types;
using namespace network::service;

namespace
{
#undef  LOG
#define LOG(text) Logger::getInstance().log(EModule::SERVER, (text));
}

template <typename MessageType>
Server<MessageType>::Server() noexcept
        : entry_(ios_)
        , netloop_(std::make_unique<ServerDataReceiver<MessageType>>(ios_, entry_, clients_), std::make_unique<ServerDataSender>())
{
    LOG_REGISTER_MODULE(EModule::SERVER)

    entry_.accessCode = network::types::SERVER_ACCESS_CODE;
}

template <typename MessageType>
Server<MessageType>::~Server()
{
    stop();
}

template <typename MessageType>
bool Server<MessageType>::start(std::pair<std::uint16_t, std::uint16_t> ports)
{
    for (;;)
    {
        try {
            ios_.run();
            break; // run() exited normally
        }
        catch (std::exception& e) {
            std::stringstream ss;
            ss << "error while asio::service::run() - " << e.what();
            LOG(ss.str().c_str())
            return false;
        }
    }

    if (not init(ports))
    {
        LOG("can't initiate entry sockets, restart or try another entry ports")
        return false;
    }

    netloop_.runThreads();

    {
        std::stringstream ss;
        ss << "server started on port: " << SERVER_IN_PORT << "/" << SERVER_OUT_PORT;
        LOG(ss.str().c_str())
    }

    return true;
}

template <typename MessageType>
bool Server<MessageType>::stop()
{
    netloop_.stopThreads();
    ios_.stop();

    LOG("closing sockets")
    std::for_each(std::begin(clients_.vIn), std::end(clients_.vIn), helper::closeSocket);
    std::for_each(std::begin(clients_.vOut), std::end(clients_.vOut), helper::closeSocket);
    helper::closeSocket(entry_.in);
    helper::closeSocket(entry_.out);

    LOG("server stopped")
    return true;
}

template <typename MessageType>
bool Server<MessageType>::init(std::pair<std::uint16_t, std::uint16_t> entryPorts)
{
    // server entry in/out service ports
    context_.inPort = entryPorts.first;
    context_.outPort = entryPorts.second;

    LOG("server init")
    boost::system::error_code ec;
    {
        auto s = helper::prepareSocket(ios_, ec, SERVER_IN_PORT);
        if (s.has_value())
        {
            entry_.in = std::forward<net::ip::udp::socket>(s.value());
        }
        else
        {
            std::stringstream ss;
            ss << "prepare entry in socket error [" + std::to_string(SERVER_IN_PORT) << "] " << ec.message();
            LOG(ss.str().c_str())
            return false;
        }
    }
    {
        auto s = helper::prepareSocket(ios_, ec, SERVER_OUT_PORT);
        if (s.has_value())
        {
            entry_.out = std::forward<net::ip::udp::socket>(s.value());
        }
        else
        {
            std::stringstream ss;
            ss << "prepare entry out socket error [" + std::to_string(SERVER_OUT_PORT) << "] " << ec.message();
            LOG(ss.str().c_str())
            return false;
        }
    }

    clients_.reserve(MAX_CLIENTS);

    return true;
}


