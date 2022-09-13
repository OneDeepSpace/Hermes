/*
 * Simple server for testing hermes library
 */

#include <iostream>
#include <exception>

#include "chat_type_id.h"

#include <hermes/log/log.h>
#include <hermes/common/types.h>
#include <hermes/service/server/server.h>
#include <hermes/message/message_generator.h>

using namespace app::message::id;

using namespace network::service;
using namespace network::message;
using namespace utility::logger;

namespace
{
#undef  LOG
#define LOG(text) utility::logger::Logger::getInstance().log(EModule::MAIN, (text));
}

int main()
{
    // config --- todo: add boost::program_options + {unit}.ini
    {
        const char* appName  { "server" };
        const char* p7Config { "/P7.Sink=Console /P7.On=1 /P7.Pool=1024 /P7.Format=\"{%cn}{%mn}[%tm][%lv] %ms\"" };

        Logger::init(p7Config, appName);
        LOG_REGISTER_MODULE(EModule::MAIN)
    }

    try
    {
        Server<ChatType> server;
        server.start({ SERVER_IN_PORT, SERVER_OUT_PORT });

        std::this_thread::sleep_for(std::chrono::milliseconds(10'000));
    }
    catch (std::exception& e)
    {
        std::stringstream ss;
        ss << "Caught an exception: " << e.what();
        LOG(ss.str().c_str())
    }

    // todo: boost SIGEV handler

    LOG("Hello, Hermes!")
    return 0;
}