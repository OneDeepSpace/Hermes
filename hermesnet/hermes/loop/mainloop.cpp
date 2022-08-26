
#include "mainloop.h"

#include <thread>
#include <hermes/log/log.h>

using namespace network;
using namespace utility::logger;

namespace
{
    #define LOG(text) \
            Logger::getInstance().log(EModule::LOOP, (text));
}

MainLoop::MainLoop(std::unique_ptr<IReceiver> r, std::unique_ptr<ISender> s)
    : bStopNetThreads_(false)
{
    receiver_ = std::forward<decltype(r)>(r);
    sender_ = std::forward<decltype(s)>(s);

    LOG_REGISTER_MODULE(EModule::LOOP)
}

MainLoop::~MainLoop()
{
    stopThreads();
}

bool MainLoop::runThreads()
{
    LOG("run network threads")
    inThread_ = std::thread(&MainLoop::processIncoming, this);
    outThread_ = std::thread(&MainLoop::processOutcoming, this);
    return true;
}

void MainLoop::stopThreads()
{
    bStopNetThreads_.store(true, std::memory_order::memory_order_acquire);

    inThread_.joinable() ? inThread_.join() : void(0);
    outThread_.joinable() ? outThread_.join() : void(0);

    LOG("network threads stopped")
}

void MainLoop::processIncoming()
{
    if (!receiver_)
        throw std::runtime_error("receiver not initialized");

    while(!bStopNetThreads_)
    {
        receiver_->process();

        // ***** DEV ONLY *****
        LOG("tick net::in_process")
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        // ********************
    }

    LOG("end of net::in_process loop")
}

void MainLoop::processOutcoming()
{
    if (!sender_)
        throw std::runtime_error("sender not initialized");

    while(!bStopNetThreads_)
    {
        sender_->process();

        // ***** DEV ONLY *****
        LOG("tick net::out_process")
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        // ********************
    }

    LOG("end of net::out_process loop")
}
