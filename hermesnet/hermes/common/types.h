#pragma once

#include <cstdint>

namespace network::types
{
    //todo: move to config module
    static constexpr std::uint16_t SERVER_IN_PORT {7000};
    static constexpr std::uint16_t SERVER_OUT_PORT {7001};

    constexpr std::uint32_t MAX_CLIENTS     {100};
    constexpr std::uint32_t SOCK_BUF_SIZE   {8192}; // 64/128 messages count
}

