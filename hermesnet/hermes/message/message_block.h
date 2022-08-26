#pragma once

#include "message_id.h"
#include "message_header.h"
#include "message_body.h"

#include <hermes/log/log.h>

/*
 *
 * !!! WORK IN PROGRESS !!!
 * !!! STILL TESTING !!!
 *
 */

namespace network::message::v2
{
    // TODO: place into config
    constexpr std::uint8_t  SERVER_ACCESS_CODE      {0xEA};
    constexpr std::uint8_t  END_MESSAGE_BYTE        {0xFF};


    constexpr std::uint32_t ACCESS_BYTE_POS         {7};
    constexpr std::uint32_t END_MESSAGE_BYTE_POS    {63};

    /**
     *  Cетевое сообщение (пакет).
     *  Состоит из заголовка, описывающего тип сообщения и
     *  тела сообщения, содержащего полезную нагрузку.
     *  Размер - 64 байт
     */
    template <typename message_id_t>
    struct message_block_t
    {
        message_block_t() = default;
        // noncopyable
        message_block_t(const message_block_t<message_id_t>&) = delete;
        message_block_t<message_id_t>& operator= (const message_block_t<message_id_t>&) = delete;

        message_block_t(message_block_t&& other) noexcept
        {
            std::swap(header, other.header);
            std::swap(body, other.body);
        }

        message_block_t<message_id_t>& operator= (message_block_t<message_id_t>&& other) noexcept
        {
            if (this != &other)
            {
                std::swap(header, other.header);
                std::swap(body, other.body);
            }
            return *this;
        }

        // ---------------------------------------------------------------------------------------
        message_header_t<message_id_t>  header {};  // [10 byte]
        message_body_t                  body   {};  // [54 bytes]
        // ---------------------------------------------------------------------------------------

        template <typename data_t>
        message_body_t::rw_result_t
        insert(data_t& src) noexcept
        {
            return body.write(src, sizeof(data_t));
        }

        template <typename data_t>
        message_body_t::rw_result_t
        insert(data_t& src, std::size_t n) noexcept
        {
            return body.write(src, n);
        }

        template <typename data_t>
        message_body_t::rw_result_t
        extract(data_t& dst) noexcept
        {
            return body.read(dst, sizeof(data_t));
        }

        template <typename data_t>
        message_body_t::rw_result_t
        extract(data_t& dst, std::size_t n) noexcept
        {
            return body.read(dst, n);
        }

        [[nodiscard]]
        std::size_t get_payload_size() const
        {
            return body.size_;
        }

        friend std::ostream& operator<< (std::ostream& os, const message_block_t<message_id_t>& msg)
        {
            os << "********** message **********\n"
               << msg.header
               << msg.body
               << "*****************************\n";
            return os;
        }
    }; // message_block_t

}   // network::message::v2

