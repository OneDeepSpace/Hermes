
#include "server.h"

#include <memory>

#include <hermes/message/message_block.h>

#include <hermes/log/log.h>
#include <hermes/common/types.h>
#include <hermes/service/helper/socket_helper.h>
#include <hermes/data_sender/server_data_sender.h>
#include <hermes/data_receiver/server_data_receiver.h>

using namespace utility::logger;
using namespace network::types;
using namespace network::service;

namespace
{
    #define LOG(text) \
            Logger::getInstance().log(EModule::SERVER, (text));
}

Server::Server() noexcept
    : entry_(ios_)
    , netloop_(std::make_unique<ServerDataReceiver>(ios_, entry_, clients_), std::make_unique<ServerDataSender>())
{
    LOG_REGISTER_MODULE(EModule::SERVER)

    entry_.accessCode = network::message::v2::SERVER_ACCESS_CODE;
}

Server::~Server()
{
    stop();
}

bool Server::start(std::pair<std::uint16_t, std::uint16_t> ports)
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

    init(ports);
    netloop_.runThreads();

    {
        std::stringstream ss;
        ss << "server started on port: " << SERVER_IN_PORT << "/" << SERVER_OUT_PORT;
        LOG(ss.str().c_str())
    }

    return true;
}

bool Server::stop()
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

bool Server::init(std::pair<std::uint16_t, std::uint16_t> entryPorts)
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



