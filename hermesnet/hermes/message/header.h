
#pragma once

#include <cstdint>
#include <iomanip>
#include <iostream>

namespace network::message
{

    /* ------------- Net message header --------------
     *
     * Contains specific type describing handle logic,
     * access code for client (unique for each client),
     * message uuid (need only for combined messages),
     * encode and decode flags.
     *
     * Structure size - 10 bytes
     * ----------------------------------------------- */

    template <typename IdType>
    struct Header
    {
        // -----------------------------------------------------------------------------
        IdType          type        {};         // message type id              [4 byte]
        std::uint8_t    uuid        { 0 };      // unique message id            [1 byte]
        std::uint8_t    block_num   { 1 };      // message block number         [1 byte]
        std::uint8_t    block_count { 1 };      // message block count          [1 byte]
        std::uint8_t    access_code { 0x0 };    // endpoint session access code [1 byte]
        bool            encode      { false };  // message encode flag          [1 byte]
        bool            compress    { false };  // message compress flag        [1 byte]
        // -----------------------------------------------------------------------------

        Header() = default;
        ~Header() = default;
        // noncopyable
        Header(Header<IdType> const&) = delete;
        Header<IdType>& operator= (Header<IdType> const&) = delete;
        // move semantic
        Header(Header<IdType>&&) noexcept;
        Header<IdType>& operator= (Header<IdType>&&) noexcept;

        template<class U>
        friend std::ostream& operator << (std::ostream& os, Header<IdType> const& h);

        void swap(Header& b) noexcept;

    };  // Header

    // ********************************* IMPLEMENTATION **********************************

    template <typename IdType>
    Header<IdType>::Header(Header<IdType>&& other) noexcept
    {
        this != &other ? this->swap(other) : void(0);
    }

    template <typename IdType>
    Header<IdType>& Header<IdType>::operator= (Header<IdType> &&other) noexcept
    {
        this != &other ? this->swap(other) : void(0);
        return *this;
    }

    template <typename IdType>
    std::ostream& operator<< (std::ostream& os, Header<IdType> const& h) {

        auto asHex = [&os](std::uint8_t b) -> void {
            os  << "0x" << std::hex << std::setw(2) << std::setfill('0')
                << std::uppercase << static_cast<unsigned>(b) << std::dec;
        };

        auto asInt = [&os](std::uint8_t b) -> void {
            os << std::dec << static_cast<unsigned>(b);
        };

        os  << "header:\n"
            << "  type        - " << h.type << "\n"
            << "  uuid        - " ; asHex(h.uuid); os << "\n"
            << "  block_num   - " ; asInt(h.block_num); os << "\n"
            << "  block_count - " ; asInt(h.block_count); os << "\n"
            << "  access_code - " ; asHex(h.access_code); os << "\n"
            << "  encode      - " << (h.encode ? "true" : "false") << "\n"
            << "  compress    - " << (h.compress ? "true" : "false") << "\n";
        return os;
    }

    template <typename IdType>
    void Header<IdType>::swap(Header<IdType> &b) noexcept
    {
        std::swap(type, b.type);
        std::swap(uuid, b.uuid);
        std::swap(block_num, b.block_num);
        std::swap(block_count, b.block_count);
        std::swap(access_code, b.access_code);
        std::swap(encode, b.encode);
        std::swap(compress, b.compress);
    }

}   // network::message