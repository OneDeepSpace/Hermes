#pragma once

#include <thread>
#include <atomic>
#include <cstdlib>
#include <optional>

#include <boost/noncopyable.hpp>

#include <hermes/data_sender/interface/isender.h>
#include <hermes/data_receiver/interface/ireceiver.h>

using namespace network::service;

namespace network
{
    /*
     *  Цикл обработки сетевых сообщений
     */
    class NetLoop : boost::noncopyable
    {
    private:
        std::thread         inThread_, outThread_;
        std::atomic_bool    bStopNetThreads_;

        std::unique_ptr<IReceiver>  receiver_   { nullptr };
        std::unique_ptr<ISender>    sender_     { nullptr };

    public:
        explicit NetLoop(std::unique_ptr<IReceiver> r, std::unique_ptr<ISender> s);
        virtual ~NetLoop();

        // todo: better return value
        bool runThreads();
        void stopThreads();

    private:
        // Принять и обработать входящие сообщения
        void processIncoming();
        // Подготовить и отправить сообщения клиентам
        void processOutcoming();

    };  // NetLoop

}   // namespace network

