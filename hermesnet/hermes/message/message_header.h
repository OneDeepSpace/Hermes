#pragma once

#include <ostream>
#include <cstdint>
#include <algorithm>

namespace network::message::v2
{
    /**
     *  Заголовок сетевого сообщения. Содержит тип отправляемого
     *  сообщения, код доступа, флаги кодирования и сжатия
     *  Размер - 10 байт
     */
    template <typename message_id_t>
    struct message_header_t
    {
        message_header_t() = default;
        // noncopyable
        message_header_t(const message_header_t<message_id_t>&) = delete;
        message_header_t<message_id_t>& operator= (const message_header_t<message_id_t>&) = delete;

        message_header_t(message_header_t<message_id_t>&& other) noexcept
        {
            std::swap(type, other.type);
            std::swap(uuid, other.uuid);
            std::swap(block_num, other.block_num);
            std::swap(block_count, other.block_count);
            std::swap(access_code, other.access_code);
            std::swap(encode, other.encode);
            std::swap(compress, other.compress);
        }

        message_header_t<message_id_t>& operator= (message_header_t<message_id_t>&& other) noexcept
        {
            if (this != &other)
            {
                std::swap(type, other.type);
                std::swap(uuid, other.uuid);
                std::swap(block_num, other.block_num);
                std::swap(block_count, other.block_count);
                std::swap(access_code, other.access_code);
                std::swap(encode, other.encode);
                std::swap(compress, other.compress);
            }
            return *this;
        }

        // ---------------------------------------------------------------------------------------
        message_id_t    type        {};         // message type id              [4 byte]
        std::uint8_t    uuid        {0};        // unique message id            [1 byte]
        std::uint8_t    block_num   {1};        // message block number         [1 byte]
        std::uint8_t    block_count {1};        // message block count          [1 byte]
        std::uint8_t    access_code {0x0};      // endpoint session access code [1 byte]
        bool            encode      {false};    // message encode flag          [1 byte]
        bool            compress    {false};    // message compress flag        [1 byte]
        // ---------------------------------------------------------------------------------------

        friend std::ostream& operator << (std::ostream& os, const message_header_t<message_id_t>& header)
        {
            os << std::hex << "header:"
               << "\n\ttype        - " << header.type
               << "\n\tuuid        - " << header.uuid
               << "\n\tblock_num   - " << header.block_num
               << "\n\tblock_count - " << header.block_count
               << "\n\taccess_code - " << header.access_code
               << "\n\tencode      - " << (header.encode ? "true" : "false")
               << "\n\tcompress    - " << (header.compress ? "true" : "false")
               << "\n";
            return os;
        }
    }; // message_header_t
}   // network::message::v2