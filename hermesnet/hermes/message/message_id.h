#pragma once

#include <ostream>
#include <cstdint>
#include <algorithm>



namespace network::message::v2
{


    /**
     *  Сервисные сообщения, общие для любого приложения.
     *
     *  Размер - 4 байта
     */
    struct service_type
    {
        service_type() = default;
        // noncopyable
        service_type(const service_type&) = delete;
        service_type& operator= (const service_type&) = delete;

        service_type(service_type&& other) noexcept
            : action(action_t::NONE)
            , reserved(reserved_t::NONE)
        {
            std::swap(action, other.action);
            std::swap(reserved, other.reserved);
        }

        service_type& operator= (service_type&& other) noexcept
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

        friend std::ostream& operator<< (std::ostream& os, const service_type& id) {
            std::string s;

            os << std::hex << "[ action - ";
            switch(id.action)
            {
                case action_t::PING:            os << "PING";           break;
                case action_t::CONNECT:         os << "CONNECT";        break;
                case action_t::DISCONNECT:      os << "DISCONNECT";     break;
                default: os << "unknown!!!";
            }

            return os << s << " ]";
        }

    };  // service_type

    /**
     *  Сообщения чат-мессенджера
     *  TODO: должны быть в папке проекта
     *
     *  Размер - 4 байта
     */
    struct chat_app_type
    {
        chat_app_type() = default;
        // noncopyable
        chat_app_type(const chat_app_type&) = delete;
        chat_app_type& operator= (const chat_app_type&) = delete;

        chat_app_type(chat_app_type&& other) noexcept
            : action(action_t::NONE)
            , reserved(reserved_t::NONE)
        {
            std::swap(action, other.action);
            std::swap(reserved, other.reserved);
        }

        chat_app_type& operator= (chat_app_type&& other) noexcept
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

        friend std::ostream& operator<< (std::ostream& os, const chat_app_type& id) {
            std::string s;

            os << std::hex << "[ action - ";
            switch(id.action)
            {
                case action_t::CHAT_MESSAGE:    os << "CHAT_MESSAGE";   break;
                default: os << "unknown!!!";
            }

            return os << s << " ]";
        }
    }; // chat_app_type

}   // network::message::v2
