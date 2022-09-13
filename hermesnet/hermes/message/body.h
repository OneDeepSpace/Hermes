
#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <algorithm>

#include <hermes/common/memory.h>

namespace network::message
{
    static constexpr std::size_t CAPACITY { 51 };   // размер полезной нагрузки

    /* ------------- Net message body ------------
     *
     * Contains payload size and continuous buffer
     * for it. Payload limit for one msg is 51 byte,
     * last byte should be marked 0xff as finish flag.
     *
     * Structure size - 54 bytes
     * ------------------------------------------- */

    struct Body
    {
        using SizeType   = std::uint16_t;
        using BufferType = std::array<std::uint8_t, CAPACITY + 1>;

        // ------------------------------------------------------------
        SizeType    size { 0 }; // current payload size      [2 bytes]
        BufferType  buf  { 0 }; // raw bytes                 [52 bytes]
        // ------------------------------------------------------------

        Body() = default;
        ~Body() = default;
        // noncopyable
        Body(Body const&) = delete;
        Body& operator= (Body const&) = delete;
        // move semantic
        Body(Body&&) noexcept;
        Body& operator= (Body&&) noexcept;

        friend std::ostream& operator<< (std::ostream& os, Body const& b);

        /* Read/Write operation result
         *  bool        - operation success flag
         *  std::size_t - processed byte count
         *  std::size_t - available buffer space
         */
        using WriteRes = std::tuple<bool, std::size_t, std::size_t>;
        using ReadRes  = std::tuple<bool, std::size_t, std::size_t>;

        template <typename Data>
        inline ReadRes read(Data& dst, std::size_t n) noexcept;
        template <typename Data>
        inline WriteRes write(Data& src, std::size_t n) noexcept;

        void swap(Body& b) noexcept;

    };  // Body

    // ********************************* IMPLEMENTATION **********************************

    Body::Body(Body &&other) noexcept
    {
        this != &other ? this->swap(other) : void(0);
    }

    Body& Body::operator= (Body &&other) noexcept
    {
        this != &other ? this->swap(other) : void(0);
        return *this;
    }

    template <typename Data>
    inline Body::Body::WriteRes Body::write(Data &src, std::size_t n) noexcept
    {
        std::size_t free { CAPACITY - size };
        assert(n <= free);

        memory::memcpy(&buf + size, &src, n);
        size += n;
        free -= n;

        return { true, n, free };
    }

    template <typename Data>
    inline Body::ReadRes Body::read(Data &dst, std::size_t n) noexcept
    {
        assert(n <= size);

        memory::memcpy(&dst, &buf + size - n, n);
        size -= n;

        return { true, n, (CAPACITY - size) };
    }

    std::ostream& operator<< (std::ostream& os, Body const& b) {

        auto asHex = [&os](std::uint8_t b) -> void {
            os  << "0x" << std::hex << std::setw(2) << std::setfill('0')
            << std::uppercase << static_cast<unsigned>(b) << ' ' << std::dec;
        };

        os  << "body:\n"
            << "  size - " << b.size << "\n"
            << "  data - ";
        std::for_each(std::begin(b.buf), std::end(b.buf), asHex);
        os << "\n";
        return os;
    }

    void Body::swap(Body &b) noexcept
    {
        std::swap(size, b.size);
        std::swap(buf, b.buf);
    }

}   // network::message
