#pragma once

#include "modules.h"
#include <iostream>
#include <string_view>
#include <unordered_map>

#include <P7_Version.h>
#include <P7_Client.h>
#include <P7_Trace.h>
#include <P7_Extensions.h>

/*
 * TODO list:
 *  - print format
 *  - severity/verbose level into LOG method
 *  - overload operator<<
 *  - helper make log string function / template log method
 */

#define LOG_REGISTER_MODULE(module) \
    utility::logger::Logger::getInstance().registerModule((module));

namespace utility::logger
{
    class P7ClientWrapper
    {
    private:
        IP7_Client* ptr_ { nullptr };

    public:
        P7ClientWrapper() = default;
        explicit P7ClientWrapper(const char* pArgs) noexcept;
        virtual ~P7ClientWrapper();

        [[nodiscard]] IP7_Client* get() const;
    };

    class IP7TraceWrapper
    {
    private:
        using hash_map = std::unordered_map<EModule, IP7_Trace::hModule>;

        IP7_Trace*  pTrace_  { nullptr };
        hash_map    modules_;

    public:
        IP7TraceWrapper() = default;
        explicit IP7TraceWrapper(IP7_Client* client, const char* name) noexcept;
        virtual ~IP7TraceWrapper();

    public:

        bool registerModule(EModule name) noexcept;
        void log(EModule name, char const* text) noexcept;

        [[nodiscard]] IP7_Trace* get() const;
    };

    class Logger
    {
    private:
        std::string settings_ {};
        P7ClientWrapper client_;
        IP7TraceWrapper trace_;

        explicit Logger(std::string* const s, std::string const& name);
        static Logger& getInstanceImpl(std::string* const s = nullptr, std::string const& n = "app");

    public:
        static Logger& getInstance();
        virtual ~Logger();

        static void init(std::string settings, std::string const& name);

        Logger(Logger&) = delete;
        Logger(Logger&&) = delete;
        Logger& operator=(Logger&) = delete;
        Logger& operator=(Logger&&) = delete;

    public:
        bool registerModule(EModule name) noexcept;
        void log(EModule module, char const* text) noexcept;

    };  // Logger

}   // utility::logger

