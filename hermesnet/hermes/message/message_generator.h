#pragma once

#include "message_block.h"

namespace network::message::v2
{
    /*
     *  Net message builder
     */
    template<typename MessageTypeId>
    class MessageGenerator
    {
    public:
        // Сгенерировать сообщение(-я) в зависимости от Типа сообщений приложения и размера полезной нагрузки.
        // Если размер payload'a больше чем может вместить одно сообщение - создаётся необходимое
        // количество сетевых пакетов для отправки.
        template <typename PayloadType>
        static std::vector<message_block_t<MessageTypeId>>
        generate(MessageTypeId typeId, PayloadType payload, std::size_t len) noexcept;



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
