#pragma once

#include <vector>
#include <thread>
#include <atomic>
#include <cstdlib>
#include <optional>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

#include <hermes/common/structures.h>
#include <hermes/data_sender/sender.h>
#include <hermes/data_receiver/receiver.h>

namespace network
{
#ifndef ASIO_TYPEDEF
#define ASIO_TYPEDEF
    namespace net = boost::asio;
#endif

    /*
     *  Цикл обработки сетевых сообщений
     */
    class MainLoop : boost::noncopyable
    {
    private:
        std::thread         inThread_, outThread_;
        std::atomic_bool    bStopNetThreads_;
        net::io_service     ios_;

        class DataReceiver  receiver_;
        class DataSender    sender_;

        class Entry   entry_;
        class Clients clients_;

        // todo: make special buffer class for message exchange logic (read/write flags) with event class (todo too)
        std::vector<std::uint8_t> incomingBuffer;
        std::vector<std::uint8_t> outcomingBuffer;

    public:
        MainLoop();
        virtual ~MainLoop();

        // todo: better return value
        bool start();
        void stop();

    private:
        // Создать и подготовить сокет на указанном порту
        std::optional<net::ip::udp::socket> prepareSocket(std::uint16_t port);
        // Закрыть указанный сокет
        static void closeSocket(net::ip::udp::socket& sock);
        // Закрыть все сокеты
        void closeAllSockets();

        bool init();
        void run();

        // Принять и обработать входящие сообщения
        void processIncoming();
        // Подготовить и отправить сообщения клиентам
        void processOutcoming();

    };  // MainLoop

}   // namespace network

