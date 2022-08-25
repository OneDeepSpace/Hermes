#pragma once

#include <ostream>
#include <cstdint>
#include <algorithm>

// TODO:
//  - разделить на логику приложения и игровые

namespace network::message::v2
{
    /**
     *  Тип сетевого сообщения. Отвечает за логику обработки
     *  сообщения и указывает способ, которым его нужно переслать.
     *  Размер - 4 байта
     */
    struct message_id
    {
        message_id() = default;
        // noncopyable
        message_id(const message_id&) = delete;
        message_id& operator= (const message_id&) = delete;

        message_id(message_id&& other) noexcept
            : action(action_t::NONE)
            , reserved(reserved_t::NONE)
        {
            std::swap(action, other.action);
            std::swap(reserved, other.reserved);
        }

        message_id& operator= (message_id&& other) noexcept
        {
            if (this != &other)
            {
                std::swap(action, other.action);
                std::swap(reserved, other.reserved);
            }
            return *this;
        }

        // ID - тип сообщения
        enum class action_t : std::uint16_t
        {
            NONE = 0,
            PING,
            CONNECT,
            DISCONNECT,
            ACTION,
            CHAT_MESSAGE,
            // ...add more
        };

        // *** reserved field ***
        enum class reserved_t : std::uint16_t
        {
            NONE = 0
        };

        // ---------------------------------------------------------------------------------------
        action_t    action;         // action id                    [2 byte]
        reserved_t  reserved;       // reserved field               [2 byte]
        // ---------------------------------------------------------------------------------------

        friend std::ostream& operator<< (std::ostream& os, const message_id& id) {
            std::string s;

            os << std::hex << "[ action - ";
            switch(id.action)
            {
                case action_t::PING:            os << "PING";           break;
                case action_t::CONNECT:         os << "CONNECT";        break;
                case action_t::DISCONNECT:      os << "DISCONNECT";     break;
                case action_t::ACTION:          os << "ACTION";         break;
                case action_t::CHAT_MESSAGE:    os << "CHAT_MESSAGE";   break;
                default: os << "unknown!!!";
            }

            return os << s << " ]";
        }
    }; // message_id

}   // network::message::v2
