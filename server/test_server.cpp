/*
 * Simple server for testing hermes library
 */

#include <iostream>
#include <exception>
#include <hermes/log/log.h>
#include <hermes/loop/mainloop.h>

using namespace network;
using namespace utility::logger;

namespace
{
#define LOG(text) \
    utility::logger::Logger::getInstance().log(EModule::MAIN, (text));
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
        MainLoop loop;
        if (loop.start())
        {
            std::string l {"server started on port - "};
            l.append(std::to_string(SERVER_IN_PORT));
            LOG(l.c_str())
        }

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