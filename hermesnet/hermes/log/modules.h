#pragma once

#include <cstdint>

namespace utility::logger
{
    enum class EModule : std::uint8_t
    {
        MAIN = 0,
        SERVER,
        CLIENT,
        LOOP,
        RECEIVER,
        SENDER,
        MESSAGE     // temp !!!
        // add more..
    };

    // helper function
    inline const char* getModuleName(EModule e) noexcept {
        switch (e) {
            case EModule::MAIN:     return "main";
            case EModule::SERVER:   return "server";
            case EModule::CLIENT:   return "client";
            case EModule::LOOP:     return "loop";
            case EModule::RECEIVER: return "receiver";
            case EModule::SENDER:   return "sender";
            case EModule::MESSAGE:  return "message";
                // add more..
            default: return "undefined";
        }
    }
}
