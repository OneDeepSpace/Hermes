#pragma once

#include <hermes/common/memory.h>

#include <sstream>
#include <cstdint>
#include <algorithm>

#include <hermes/log/log.h>

// todo:
//  - human readable message byte fields !
//  - helper functions for serialize/deserialize message payloads (make ping/action/chatmsg/conn/disc/etc)
//  - objects for message payloads/actions

using namespace utility::logger;

namespace
{
    #define LOG_(text) \
        Logger::getInstance().log(EModule::MESSAGE, (text));
}

namespace network::message::v2
{
    static constexpr std::size_t PAYLOAD_LIMIT      {52};   // размер полезной нагрузки
    static constexpr std::size_t MESSAGE_SIZE       {64};   // размер сетевого пакета

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
            os << std::hex
               << "data:\n"
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

}   // network::message::v2
