#pragma once

#include "message_block.h"

namespace network::message::v2
{
    constexpr std::uint8_t  SERVER_ACCESS_CODE      {0xEA};
    constexpr std::uint8_t  END_MESSAGE_BYTE        {0xFF};
    constexpr std::uint32_t ACCESS_BYTE_POS         {7};
    constexpr std::uint32_t END_MESSAGE_BYTE_POS    {63};

    /*
     *  Net message factory
     */
    class MessageGenerator
    {
    public:
        template <typename T>
        static void generate();
    };

//    /**
//     * Подготовить сетевой пакет к отправке установив байты
//     * доступа к серверу и конца сообщения.
//     */
//    static inline void setMessageValidateBytes(message_block_t<message_id>& block) {
//        // todo: !!!
//        std::memset(&block + ACCESS_BYTE_POS, SERVER_ACCESS_CODE, 1);
//        std::memset(&block + END_MESSAGE_BYTE_POS, END_MESSAGE_BYTE, 1);
//    }

}
