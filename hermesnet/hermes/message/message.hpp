#pragma once

#include <cstdint>
#include <hermes/log/log.h>
#include <hermes/common/types.h>
#include <hermes/common/memory.h>

// todo:
//  - human readable message byte fields !
//  - helper functions for serialize/deserialize message payloads (make ping/action/chatmsg/conn/disc/etc)
//  - objects for message payloads/actions

/*
 *
 * !!! WORK IN PROGRESS !!!
 * !!! STILL TESTING !!!
 *
 */



using namespace utility::logger;

namespace
{
    /*
     * todo: !!! TEMP, REMOVE AFTER TESTING !!!
     */

    #define LOG_(text) \
                Logger::getInstance().log(EModule::MESSAGE, (text));
}

namespace network::message
{
    constexpr std::uint8_t  SERVER_ACCESS_CODE      {0xEA};
    constexpr std::uint8_t  END_MESSAGE_BYTE        {0xFF};
    constexpr std::uint32_t ACCESS_BYTE_POS         {7};
    constexpr std::uint32_t END_MESSAGE_BYTE_POS    {63};

    static constexpr std::size_t PAYLOAD_LIMIT      {52};   // размер полезной нагрузки
    static constexpr std::size_t MESSAGE_SIZE       {64};   // размер сетевого пакета

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
            CHAT_MESSAGE
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

            os << "[ action - ";
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
            os << "header:"
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

    /**
     *  Тело сетевого сообщения. Обёртка над std::array
     *  с операциями чтения и записи в буфер сообщения.
     *  Размер - 54 байт
     */
    struct message_body_t
    {
        message_body_t() = default;
        // noncopyable
        message_body_t(const message_body_t&) = delete;
        message_body_t& operator= (const message_body_t&) = delete;

        message_body_t(message_body_t&& other) noexcept
        {
            std::swap(size_, other.size_);
            std::swap(buf_, other.buf_);
        }

        message_body_t& operator= (message_body_t&& other) noexcept
        {
            if (this != &other)
            {
                std::swap(size_, other.size_);
                std::swap(buf_, other.buf_);
            }
            return *this;
        }

        using size_t_  = std::uint16_t;
        using array_t_ = std::array<std::uint8_t, PAYLOAD_LIMIT>;

        /** Результат чтения/записи.
         *  success - bool - флаг успешности операции
         *  read/write - std::size_t - количество обработанных байт
         *  free_space - std::size_t - количество свободных байт в буфере сообщения
         */
        using rw_result_t = std::tuple<bool, std::size_t, std::size_t>;

        // ---------------------------------------------------------------------------------------
        size_t_  size_ {0}; // size of payload          [2 bytes]
        array_t_ buf_  {0}; // raw bytes                [52 bytes] - PAYLOAD_LIMIT
        // ---------------------------------------------------------------------------------------

        template <typename data_t>
        inline rw_result_t write(data_t& src, std::size_t count) noexcept
        {
            {
                std::stringstream ss;
                ss << "[msg|body|write] " << count << " bytes into buffer";
                LOG_(ss.str().c_str())
            }
            const std::size_t END_MSG_BYTE_SIZE {1};
            const bool check {(PAYLOAD_LIMIT - END_MSG_BYTE_SIZE - size_) >= count};
#ifdef DEBUG
            // TODO: cout -> log
                assert(check);
#else
            if (!check) {
                std::stringstream ss;
                ss << "[msg|body|write] error: bad_size [" << count << " > " << size_ << "]";
                LOG_(ss.str().c_str())
                return {false, 0, PAYLOAD_LIMIT - END_MSG_BYTE_SIZE - size_};
            }
#endif

            const std::size_t offset = size_;
            memory::memcpy(buf_.data() + offset, &src, count);
            size_ += count;
            const std::size_t free_space { PAYLOAD_LIMIT - END_MSG_BYTE_SIZE - size_ };

            return {true, count, free_space};
        }

        template <typename data_t>
        inline rw_result_t read(data_t& dst, std::size_t count) noexcept
        {
            {
                std::stringstream ss;
                ss << "[msg|body|read] " << count << " bytes from buffer";
                LOG_(ss.str().c_str())
            }
            const std::size_t END_MSG_BYTE_SIZE {1};
            const bool check { size_ >= count };
#ifdef DEBUG
            assert(check);
#else
            if (!check) {
                std::stringstream ss;
                ss << "[msg|body|read] error: bad_size [" << count << " > " << size_ << "]";
                LOG_(ss.str().c_str())
                return {false, 0, PAYLOAD_LIMIT - size_};
            }
#endif

            const std::size_t offset = size_ - count;
            memory::memcpy(&dst, buf_.data() + offset, count);
            std::memset(buf_.data() + offset, 0x0, count);
            size_ -= count;
            const std::size_t free_space { PAYLOAD_LIMIT - END_MSG_BYTE_SIZE - size_ };
            return {true, count, free_space};
        }

        friend std::ostream& operator<< (std::ostream& os, const message_body_t& body)
        {
            std::ios_base::fmtflags f(os.flags());
            os << "data:\n"
               << "\tsize - " << body.size_ << ", free - " << (PAYLOAD_LIMIT - body.size_) << "\n"
               << "\tpayload - ";
            if (!body.size_)
            {
                os << "<empty>";
            }
            else
            {
                for (auto& elem: body.buf_) os << elem << " ";
            }
            os << std::hex << '\n';
            os.flags(f);

            return os;
        }
    }; // message_body_t

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

    /**
     * Подготовить сетевой пакет к отправке установив байты
     * доступа к серверу и конца сообщения.
     */
    static inline void setMessageValidateBytes(message_block_t<message_id>& block) {
        // todo: !!!
        std::memset(&block + ACCESS_BYTE_POS, SERVER_ACCESS_CODE, 1);
        std::memset(&block + END_MESSAGE_BYTE_POS, END_MESSAGE_BYTE, 1);
    }

}
