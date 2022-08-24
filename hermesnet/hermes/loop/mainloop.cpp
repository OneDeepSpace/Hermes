
#include "mainloop.h"

#include <thread>
#include <hermes/log/log.h>

using namespace network;
using namespace utility::logger;

namespace
{   // right local LOG macro for this entire module
    #define LOG(text) \
            Logger::getInstance().log(EModule::LOOP, (text));
}

MainLoop::MainLoop()
    : entry_(ios_)
    , bStopNetThreads_(false)
{
    LOG_REGISTER_MODULE(EModule::LOOP)
    LOG_REGISTER_MODULE(EModule::MESSAGE) // temp
}

MainLoop::~MainLoop()
{
    stop();
}

// todo: return value
bool MainLoop::start()
{
    if (init())
    {
        run();
        return true;
    }
    return false;
}

void MainLoop::stop()
{
    bStopNetThreads_.store(true, std::memory_order::memory_order_acquire);

    ios_.stop();

    inThread_.joinable() ? inThread_.join() : void(0);
    outThread_.joinable() ? outThread_.join() : void(0);

    closeAllSockets();

    LOG("stopped")
}

std::optional<net::ip::udp::socket>
MainLoop::prepareSocket(std::uint16_t port)
{
    const std::string& address = "127.0.0.1";   // todo: get local ip address
    net::ip::udp::endpoint endpoint(net::ip::address::from_string(address), port);
    net::ip::udp::socket sock(ios_);

    boost::system::error_code ec;
    sock.open(endpoint.protocol(), ec);
    if (ec.failed() or !sock.is_open()) {
        std::stringstream ss;
        ss << "prepare socket | error while opening socket - " << ec.message();
        LOG(ss.str().c_str())
        return std::nullopt;
    }

    sock.bind(endpoint, ec);
    if (ec.failed()) {
        std::stringstream ss;
        ss << "prepare socket | error while binding socket - " << ec.message();
        LOG(ss.str().c_str())
        return std::nullopt;
    }

    sock.non_blocking(true);

    sock.set_option(net::ip::udp::socket::send_buffer_size(SOCK_BUF_SIZE));
    sock.set_option(net::ip::udp::socket::receive_buffer_size(SOCK_BUF_SIZE));
    sock.set_option(net::ip::udp::socket::reuse_address(true));
    const std::uint32_t dummyBoostTemplateParam {0};
    sock.set_option(net::ip::udp::socket::linger(false, dummyBoostTemplateParam));

    {   // log scope
        std::stringstream str;
        str << "prepare socket | init successful [" << sock.local_endpoint() << "]";
        LOG(str.str().c_str())
    }   // log scope

    return sock;
}

void MainLoop::closeSocket(net::ip::udp::socket& sock)
{
    if (sock.is_open())
    {
        // close gracefully
        boost::system::error_code ec;
        // looks like not for the UDP overall ¯\_(ツ)_/¯
        sock.shutdown(net::ip::udp::socket::shutdown_both, ec);
        sock.close();
    }
}

void MainLoop::closeAllSockets()
{
    LOG("close sockets")
    std::for_each(std::begin(clients_.vIn), std::end(clients_.vIn), closeSocket);
    std::for_each(std::begin(clients_.vOut), std::end(clients_.vOut), closeSocket);
    closeSocket(entry_.in);
    closeSocket(entry_.out);
}

bool MainLoop::init()
{
    LOG("init")
    {
        auto s = prepareSocket(SERVER_IN_PORT);
        if (s.has_value())
            entry_.in = std::forward<net::ip::udp::socket>(s.value());
        else
        {
            std::stringstream ss;
            ss << "error while create entry IN socket on port: " << std::to_string(SERVER_IN_PORT);
            LOG(ss.str().c_str())
            return false;
        }
    }
    {
        auto s = prepareSocket(SERVER_OUT_PORT);
        if (s.has_value())
            entry_.out = std::forward<net::ip::udp::socket>(s.value());
        else
        {
            std::stringstream ss;
            ss << "error while create entry IN socket on port: " + std::to_string(SERVER_OUT_PORT);
            LOG(ss.str().c_str())
            return false;
        }
    }

    clients_.reserve(MAX_CLIENTS);

    return true;
}

void MainLoop::run()
{
    LOG("start")
    for (;;)
    {
        try {
            ios_.run();
            inThread_ = std::thread(&MainLoop::processIncoming, this);
            outThread_ = std::thread(&MainLoop::processOutcoming, this);
            break; // run() exited normally
        }
        catch (std::exception& e) {
            std::stringstream logStr;
            logStr << "error while run() asio::service - " << e.what();
            LOG(logStr.str().c_str())
        }
    }
}

void MainLoop::processIncoming()
{
    while(!bStopNetThreads_)
    {
        receiver_.processIncomingMessages(entry_, clients_, incomingBuffer);

        // ***** DEV ONLY *****
        LOG("tick net::in_process")
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        // ********************
    }

    LOG("end of net::in_process loop")
}

void MainLoop::processOutcoming()
{
    while(!bStopNetThreads_)
    {
        sender_.processOutcomingMessages(entry_, clients_, outcomingBuffer);

        // ***** DEV ONLY *****
        LOG("tick net::out_process")
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        // ********************
    }

    LOG("end of net::out_process loop")
}



