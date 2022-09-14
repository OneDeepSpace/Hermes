
#include "netloop.h"

#include <thread>
#include <hermes/log/log.h>

using namespace network;
using namespace utility::logger;

namespace
{
#undef  LOG
#define LOG(text) Logger::getInstance().log(EModule::NETLOOP, (text));
}

NetLoop::NetLoop(std::unique_ptr<IReceiver> r, std::unique_ptr<ISender> s)
    : bStopNetThreads_(false)
{
    receiver_ = std::forward<decltype(r)>(r);
    sender_ = std::forward<decltype(s)>(s);

    LOG_REGISTER_MODULE(EModule::NETLOOP)
}

NetLoop::~NetLoop()
{
    stopThreads();
}

bool NetLoop::runThreads()
{
    LOG("run network threads")
    inThread_ = std::thread(&NetLoop::processIncoming, this);
    outThread_ = std::thread(&NetLoop::processOutcoming, this);
    return true;
}

void NetLoop::stopThreads()
{
    if (not bStopNetThreads_)
    {
        bStopNetThreads_.store(true, std::memory_order::memory_order_acquire);

        inThread_.joinable()  ? inThread_.join()  : void(0);
        outThread_.joinable() ? outThread_.join() : void(0);

        LOG("network threads stopped")
    }
}

void NetLoop::processIncoming()
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

void NetLoop::processOutcoming()
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
