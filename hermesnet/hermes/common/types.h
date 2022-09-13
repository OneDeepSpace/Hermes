#pragma once

#include <cstdint>

#define SIZEOF(T) std::cout \
    << #T << ":\n\t" \
	<< "sizeof = " << sizeof(T) << "\n\t" \
	<< "alignof = " << alignof(T) << "\n";

namespace network::types
{
    // TODO: move all into config module

    static constexpr std::uint16_t SERVER_IN_PORT {7000};
    static constexpr std::uint16_t SERVER_OUT_PORT {7001};

    constexpr std::uint8_t  SERVER_ACCESS_CODE      {0xEA};
    constexpr std::uint8_t  END_MESSAGE_BYTE        {0xFF};

    constexpr std::uint32_t ACCESS_BYTE_POS         {7};
    constexpr std::uint32_t END_MESSAGE_BYTE_POS    {63};

    constexpr std::uint32_t MAX_CLIENTS     {100};
    constexpr std::uint32_t SOCK_BUF_SIZE   {8192}; // 64/128 messages count
}

